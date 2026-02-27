#include "api.h"
#include "../miniwindow/imagefilters.h"
#include "../qlua.h"
#include "../scriptapi.h"
#include "errors.h"
#include "smushclient_qt/src/ffi/client.cxxqt.h"
#include <QtCore/QElapsedTimer>
#include <QtCore/QPointer>
extern "C"
{
#include "lauxlib.h"
#include "lua.h"
}

using std::nullopt;
using std::optional;
using std::string;
using std::string_view;

using qlua::expectMaxArgs;
using qlua::push;
using qlua::pushList;
using qlua::pushQVariant;

DECLARE_ENUM_BOUNDS(Qt::Orientation, Horizontal, Vertical);
DECLARE_ENUM_BOUNDS(ffi::filter::Directions, Both, Vertical);
DECLARE_ENUM_BOUNDS(SendTarget, World, ScriptAfterOmit);
DECLARE_ENUM_BOUNDS(ExportKind, Trigger, Keypad);

namespace {
const char* const indexRegKey = "smushclient.plugin";
const char* const worldRegKey = "smushclient.world";
const char* const worldLibKey = "world";
} // namespace

#define expect_nonnull(opt, code)                                              \
  if (!(opt)) [[unlikely]] {                                                   \
    return returnCode(L, (code));                                              \
  }

namespace {
// Private localization

QString
fmtNoSuchPlugin(string_view id)
{
  return ScriptApi::tr("Plugin ID (%1) is not installed").arg(id);
}

QString
fmtPluginDisabled(const Plugin& plugin)
{
  return ScriptApi::tr("Plugin '%1' (%2) is not enabled")
    .arg(plugin.name())
    .arg(plugin.id());
}

QString
fmtNoSuchRoutine(const Plugin& plugin, string_view routine)
{
  return ScriptApi::tr("No function '%1' in plugin '%2' (%3)")
    .arg(routine)
    .arg(plugin.name())
    .arg(plugin.id());
}

QString
fmtBadParam(int idx, const char* type)
{
  return ScriptApi::tr("Cannot pass argument #%1 (%2 type) to CallPlugin")
    .arg(idx)
    .arg(type);
}

QString
fmtCallError(const Plugin& plugin, string_view routine)
{
  return ScriptApi::tr("Runtime error in function '%1', plugin '%2' (%3)")
    .arg(routine)
    .arg(plugin.name())
    .arg(plugin.id());
}

QString
fmtBadReturn(const Plugin& plugin,
             string_view routine,
             int idx,
             const char* type)
{
  return ScriptApi::tr("Cannot handle return value #%1 (%2 type) from function "
                       "'%3' in plugin '%4' (%5)")
    .arg(idx)
    .arg(type)
    .arg(routine)
    .arg(plugin.name())
    .arg(plugin.id());
}

// Private utils

inline int
returnCode(lua_State* L, ApiCode code)
{
  push(L, code);
  return 1;
}

inline int
returnCode(lua_State* L, ApiCode code, const QString& reason)
{
  push(L, code);
  push(L, reason);
  return 2;
}

inline size_t
getPluginIndex(lua_State* L)
{
  lua_rawgetp(L, LUA_REGISTRYINDEX, indexRegKey);
  const size_t index = lua_tointeger(L, -1);
  lua_pop(L, 1);
  return index;
}

inline optional<string_view>
getSenderOption(lua_State* L, int idx)
{
  switch (lua_type(L, idx)) {
    case LUA_TBOOLEAN:
      return static_cast<bool>(lua_toboolean(L, idx)) ? "1" : "0";
    case LUA_TNUMBER:
    case LUA_TSTRING:
      return qlua::toString(L, idx);
      break;
    default:
      return nullopt;
  }
}

inline void
insertTextTriples(lua_State* L, ScriptApi& api)
{
  int n = lua_gettop(L);
  for (int i = 1; i <= n; i += 3) {
    api.ColourTell(qlua::getQColor(L, i, QColor()),
                   qlua::getQColor(L, i + 1, QColor()),
                   qlua::getQString(L, i + 2));
  }
}

constexpr float
convertVolume(lua_Number decibels) noexcept
{
  return static_cast<float>(1 / pow(2, decibels / -3));
}

inline ScriptApi**
apiSlot(lua_State* L) noexcept
{
  return static_cast<ScriptApi**>(lua_getextraspace(L)); // NOLINT
}

template<typename T>
void
pushListOrEmpty(lua_State* L, const T& list)
{
  if (list.empty()) {
    lua_pushnil(L);
  } else {
    pushList(L, list);
  }
}

inline const char*
pushVariable(lua_State* L, string_view variable)
{
  if (variable.data() == nullptr) {
    lua_pushnil(L);
    return nullptr;
  }
  return push(L, variable);
}

} // namespace

int
setLuaApi(lua_State* L, ScriptApi& api)
{
  *apiSlot(L) = &api;
  return 0;
}

inline ScriptApi&
getApi(lua_State* L)
{
  return **apiSlot(L);
}

int
setPluginIndex(lua_State* L, size_t index)
{
  push(L, index);
  lua_rawsetp(L, LUA_REGISTRYINDEX, indexRegKey);
  return 0;
}

// benchmarking

#ifdef BENCHMARK_LUA
class Benchmarker
{
public:
  explicit Benchmarker(const char* name) noexcept
    : name(name)
  {
    timer.start();
  }

  ~Benchmarker()
  {
    int64_t elapsed = timer.elapsed();
    if (elapsed > 2) {
      const string_view sName = string_view(name).substr(2);
      qDebug() << QString::fromUtf8(sName) << elapsed << "ms";
    }
  }
  Benchmarker(const Benchmarker&) = default;
  Benchmarker& operator=(const Benchmarker&) = default;
  Benchmarker(Benchmarker&&) = delete;
  Benchmarker& operator=(Benchmarker&&) = delete;

private:
  const char* name;
  QElapsedTimer timer;
};

#define BENCHMARK Benchmarker benchmarker(__func__);
#else
#define BENCHMARK
#endif

namespace {

// color

int
L_AdjustColour(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  const QColor color = qlua::getQColor(L, 1);
  const optional<ColorAdjust> method = qlua::getEnum<ColorAdjust>(L, 2);
  if (!color.isValid() || !method) {
    lua_pushvalue(L, 1);
    return 1;
  }
  push(L, ScriptApi::AdjustColour(color, *method));
  return 1;
}

int
L_ColourNameToRGB(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, qlua::getQColor(L, 1));
  return 1;
}

int
L_GetBoldColour(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  const lua_Integer i = qlua::getInteger(L, 1);
  if (i >= 1 && i <= 8) {
    push(L, getApi(L).GetTermColour(static_cast<uint8_t>(i + 7)));
  } else {
    push(L, 0);
  }
  return 1;
}

int
L_GetCustomColourText(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, qlua::getCustomColor(L, 1));
  return 1;
}

int
L_GetMapColour(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  const QColor color = qlua::getQColor(L, 1);
  if (!color.isValid()) {
    lua_pushvalue(L, 1);
    return 1;
  }
  push(L, getApi(L).GetMapColour(color));
  return 1;
}

int
L_GetNormalColour(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  const lua_Integer i = qlua::getInteger(L, 1);
  if (i >= 1 && i <= 8) {
    push(L, getApi(L).GetTermColour(static_cast<uint8_t>(i - 1)));
  } else {
    push(L, 0);
  }
  return 1;
}

int
L_GetSysColor(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  const optional<SysColor> sysColor = qlua::getEnum<SysColor>(L, 1);
  expect_nonnull(sysColor, ApiCode::BadParameter);
  push(L, ScriptApi::GetSysColor(*sysColor));
  return 1;
}

int
L_MapColour(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  getApi(L).MapColour(qlua::getQColor(L, 1), qlua::getQColor(L, 2));
  return 0;
}

int
L_MapColourList(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  const QList colors = getApi(L).MapColourList();
  lua_createtable(L, 0, static_cast<int>(colors.size()));
  for (const auto& item : colors) {
    qlua::pushEntry(L, item.first, item.second);
  }
  return 1;
}

int
L_NoteColourBack(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  getApi(L).SetOption(getPluginIndex(L),
                      "note_background_colour",
                      qlua::colorToRgbCode(qlua::getQColor(L, 1)));
  return 0;
}

int
L_NoteColourFore(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  getApi(L).SetOption(getPluginIndex(L),
                      "note_text_colour",
                      qlua::colorToRgbCode(qlua::getQColor(L, 1)));
  return 0;
}

int
L_NoteColourRgb(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  ScriptApi& api = getApi(L);
  api.SetOption(getPluginIndex(L),
                "note_text_colour",
                qlua::colorToRgbCode(qlua::getQColor(L, 1)));
  api.SetOption(getPluginIndex(L),
                "note_background_colour",
                qlua::colorToRgbCode(qlua::getQColor(L, 2)));
  return 0;
}

int
L_PickColour(lua_State* L)
{
  BENCHMARK
  push(L, getApi(L).PickColour(qlua::getQColor(L, 1, QColor())));
  return 1;
}

int
L_RGBColourToName(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, qlua::rgbCodeToColor(qlua::getInteger(L, 1)).name());
  return 1;
}

int
L_SetBackgroundColour(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, getApi(L).SetBackgroundColour(qlua::getQColor(L, 1)));
  return 1;
}

int
L_SetBoldColour(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  const lua_Integer i = qlua::getInteger(L, 1);
  const QColor color = qlua::getQColor(L, 2);
  if (i >= 1 && i <= 8) {
    getApi(L).SetTermColour(static_cast<uint8_t>(i + 7), color);
  }
  return 0;
}

int
L_SetNormalColour(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  const lua_Integer i = qlua::getInteger(L, 1);
  const QColor color = qlua::getQColor(L, 2);
  if (i >= 1 && i <= 8) {
    getApi(L).SetTermColour(static_cast<uint8_t>(i - 1), color);
  }
  return 0;
}

// database

int
L_DatabaseClose(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, getApi(L).DatabaseClose(qlua::getString(L, 1)));
  return 1;
}

int
L_DatabaseOpen(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 3);
  push(L,
       getApi(L).DatabaseOpen(
         qlua::getString(L, 1), qlua::getString(L, 2), qlua::getInt(L, 3, 6)));
  return 1;
}

// info

int
L_GetInfo(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  pushQVariant(L, getApi(L).GetInfo(qlua::getInteger(L, 1)));
  return 1;
}

int
L_GetLineInfo(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  pushQVariant(
    L, getApi(L).GetLineInfo(qlua::getInt(L, 1) - 1, qlua::getInteger(L, 2)));
  return 1;
}

int
L_GetPluginInfo(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  const string_view pluginID = qlua::getString(L, 1);
  const lua_Integer infoType = qlua::getInteger(L, 2);
  if (infoType > UINT8_MAX) [[unlikely]] {
    lua_pushnil(L);
  } else {
    pushQVariant(
      L, getApi(L).GetPluginInfo(pluginID, static_cast<uint8_t>(infoType)));
  }
  return 1;
}

int
L_GetStyleInfo(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 3);
  pushQVariant(L,
               getApi(L).GetStyleInfo(qlua::getInt(L, 1),
                                      qlua::getInteger(L, 2),
                                      qlua::getInteger(L, 3)));
  return 1;
}

int
L_GetTimerInfo(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  pushQVariant(L,
               getApi(L).GetTimerInfo(getPluginIndex(L),
                                      qlua::getString(L, 1),
                                      qlua::getInteger(L, 2)));
  return 1;
}

int
L_GetUniqueNumber(lua_State* L)
{
  push(L, ScriptApi::GetUniqueNumber());
  return 1;
}

int
L_Version(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  push(L, SCRIPTING_VERSION);
  return 1;
}

int
L_WindowFontInfo(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 3);
  pushQVariant(L,
               getApi(L).WindowFontInfo(qlua::getString(L, 1),
                                        qlua::getString(L, 2),
                                        qlua::getInteger(L, 3)));
  return 1;
}

int
L_WindowImageInfo(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 3);
  pushQVariant(L,
               getApi(L).WindowImageInfo(qlua::getString(L, 1),
                                         qlua::getString(L, 2),
                                         qlua::getInt(L, 3)));
  return 1;
}

int
L_WindowInfo(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  pushQVariant(
    L, getApi(L).WindowInfo(qlua::getString(L, 1), qlua::getInteger(L, 2)));
  return 1;
}

// input

int
L_Execute(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  return returnCode(L, getApi(L).Execute(qlua::getQString(L, 1)));
}

int
L_LogSend(lua_State* L)
{
  BENCHMARK
  QByteArray bytes = qlua::concatArgs(L);
  return returnCode(L, getApi(L).LogSend(bytes));
}

int
L_Send(lua_State* L)
{
  BENCHMARK
  QByteArray bytes = qlua::concatArgs(L);
  return returnCode(L, getApi(L).Send(bytes));
}

int
L_SendImmediate(lua_State* L)
{
  BENCHMARK
  QByteArray bytes = qlua::concatArgs(L);
  return returnCode(L, getApi(L).SendImmediate(bytes));
}

int
L_SendNoEcho(lua_State* L)
{
  BENCHMARK
  QByteArray bytes = qlua::concatArgs(L);
  return returnCode(L, getApi(L).SendNoEcho(bytes));
}

int
L_SendPkt(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  return returnCode(L, getApi(L).SendPacket(qlua::getBytes(L, 1)));
}

int
L_SendPush(lua_State* L)
{
  BENCHMARK
  QByteArray bytes = qlua::concatArgs(L);
  return returnCode(L, getApi(L).SendPush(bytes));
}

// log

int
L_CloseLog(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  return returnCode(L, getApi(L).CloseLog());
}

int
L_FlushLog(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  return returnCode(L, getApi(L).FlushLog());
}

int
L_IsLogOpen(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  push(L, getApi(L).IsLogOpen());
  return 1;
}

int
L_OpenLog(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  return returnCode(
    L, getApi(L).OpenLog(qlua::getString(L, 1), qlua::getBool(L, 2, false)));
}

int
L_WriteLog(lua_State* L)
{
  BENCHMARK
  const QByteArray message = qlua::concatArgs(L);
  return returnCode(
    L, getApi(L).WriteLog(string_view(message.data(), message.size())));
}

// notepad

int
L_ActivateNotepad(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, getApi(L).ActivateNotepad(qlua::getQString(L, 1)));
  return 1;
}

int
L_AppendToNotepad(lua_State* L)
{
  BENCHMARK
  getApi(L).AppendToNotepad(qlua::getQString(L, 1),
                            QString::fromUtf8(qlua::concatArgs(L, 2)));
  push(L, 1);
  return 1;
}

int
L_CloseNotepad(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  push(
    L,
    getApi(L).CloseNotepad(qlua::getQString(L, 1), qlua::getBool(L, 2, false)));
  return 1;
}

int
L_GetNotepadLength(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, getApi(L).GetNotepadLength(qlua::getQString(L, 1)));
  return 1;
}

int
L_GetNotepadList(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  pushList(L, getApi(L).GetNotepadList(qlua::getBool(L, 1, false)));
  return 1;
}

int
L_GetNotepadText(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, getApi(L).GetNotepadText(qlua::getQString(L, 1)));
  return 1;
}

int
L_GetNotepadWindowPosition(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, getApi(L).GetNotepadWindowPosition(qlua::getQString(L, 1)));
  return 1;
}

int
L_NotepadColour(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 3);
  push(L,
       getApi(L).NotepadColour(
         qlua::getQString(L, 1), qlua::getQColor(L, 2), qlua::getQColor(L, 3)));
  return 1;
}

int
L_NotepadFont(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 5);
  const QString title = qlua::getQString(L, 1);
  const QString fontFamily = qlua::getQString(L, 2);
  const lua_Number pointSize = qlua::getNumber(L, 3);
  const QFlags<StyleFlag> styleFlags = qlua::getQFlags<StyleFlag>(L, 4);
  // const lua_Integer charset = qlua::getInteger(L, 5, 0);
  QTextCharFormat format;
  if (!title.isEmpty()) {
    format.setFontFamilies(QStringList(title));
  }
  if (pointSize != 0) {
    format.setFontPointSize(pointSize);
  }
  format.setFontWeight(styleFlags.testFlag(StyleFlag::Bold)
                         ? QFont::Weight::Bold
                         : QFont::Weight::Normal);
  format.setFontItalic(styleFlags.testFlag(StyleFlag::Italic));
  format.setFontUnderline(styleFlags.testFlag(StyleFlag::Underline));
  format.setFontStrikeOut(styleFlags.testFlag(StyleFlag::Strikeout));
  push(L, getApi(L).NotepadFont(title, format));
  return 1;
}

int
L_NotepadReadOnly(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  push(L,
       getApi(L).NotepadReadOnly(qlua::getQString(L, 1),
                                 qlua::getBool(L, 2, true)));
  return 1;
}

int
L_NotepadSaveMethod(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  const QString title = qlua::getQString(L, 1);
  const optional<Notepad::SaveMethod> method =
    qlua::getEnum<Notepad::SaveMethod>(L, 2);
  qDebug() << (method ? optional(static_cast<int64_t>(*method)) : nullopt);
  push(L, method && getApi(L).NotepadSaveMethod(title, *method));
  return 1;
}

int
L_ReplaceNotepad(lua_State* L)
{
  BENCHMARK
  push(L,
       getApi(L).ReplaceNotepad(qlua::getQString(L, 1),
                                QString::fromUtf8(qlua::concatArgs(L, 2))));
  return 1;
}

int
L_SendToNotepad(lua_State* L)
{
  BENCHMARK
  push(L,
       getApi(L).SendToNotepad(qlua::getQString(L, 1),
                               QString::fromUtf8(qlua::concatArgs(L, 2))));
  return 1;
}

// option

int
L_GetAlphaOption(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  pushVariable(
    L, getApi(L).GetAlphaOption(getPluginIndex(L), qlua::getString(L, 1)));
  return 1;
}

int
L_GetAlphaOptionList(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  pushList(L, ScriptApi::GetAlphaOptionList());
  return 1;
}

int
L_GetCurrentValue(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  pushQVariant(
    L, getApi(L).GetCurrentValue(getPluginIndex(L), qlua::getString(L, 1)));
  return 1;
}

int
L_GetOption(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, getApi(L).GetOption(getPluginIndex(L), qlua::getString(L, 1)));
  return 1;
}

int
L_GetOptionList(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  pushList(L, ScriptApi::GetOptionList());
  return 1;
}

int
L_SetAlphaOption(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  return returnCode(L,
                    getApi(L).SetAlphaOption(getPluginIndex(L),
                                             qlua::getString(L, 1),
                                             qlua::getString(L, 2)));
}

int
L_SetOption(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  return returnCode(L,
                    getApi(L).SetOption(getPluginIndex(L),
                                        qlua::getString(L, 1),
                                        qlua::getIntegerOrBool(L, 2, 0)));
}

// output

int
L_ActivateClient(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  getApi(L).ActivateClient();
  push(L, 1);
  return 1;
}

int
L_AddFont(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  return returnCode(L, ScriptApi::AddFont(qlua::getQString(L, 1)));
}

int
L_AnsiNote(lua_State* L)
{
  BENCHMARK
  const QByteArray note = qlua::concatArgs(L);
  getApi(L).AnsiNote(string_view(note.data(), note.size()));
  return 0;
}

int
L_ColourNote(lua_State* L)
{
  BENCHMARK
  ScriptApi& api = getApi(L);
  insertTextTriples(L, api);
  api.finishNote();
  return 0;
}

int
L_ColourTell(lua_State* L)
{
  BENCHMARK
  insertTextTriples(L, getApi(L));
  return 0;
}

int
L_GetLinesInBufferCount(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  push(L, getApi(L).GetLinesInBufferCount());
  return 1;
}

int
L_Hyperlink(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 7);
  getApi(L).Hyperlink(qlua::getQString(L, 1),
                      qlua::getQString(L, 2),
                      qlua::getQString(L, 3),
                      qlua::getQColor(L, 4),
                      qlua::getQColor(L, 5),
                      qlua::getBool(L, 6, false),
                      qlua::getBool(L, 7, false));
  return 0;
}

int
L_Note(lua_State* L)
{
  BENCHMARK
  ScriptApi& api = getApi(L);
  api.Tell(QString::fromUtf8(qlua::concatArgs(L)));
  api.finishNote();
  return 0;
}

int
L_SetBackgroundImage(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  const QString path = qlua::getQString(L, 1);
  const optional<MiniWindow::Position> position =
    qlua::getEnum<MiniWindow::Position>(L, 2);
  expect_nonnull(position, ApiCode::BadParameter);
  return returnCode(L, getApi(L).SetBackgroundImage(path, *position));
}

int
L_SetClipboard(lua_State* L)
{
  BENCHMARK
  ScriptApi::SetClipboard(QString::fromUtf8(qlua::concatArgs(L)));
  return 0;
}

int
L_SetCursor(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  const optional<Qt::CursorShape> cursor =
    qlua::getCursor(L, 1, Qt::CursorShape::ArrowCursor);
  expect_nonnull(cursor, ApiCode::BadParameter);
  return returnCode(L, getApi(L).SetCursor(*cursor));
}

int
L_SetForegroundImage(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  const QString path = qlua::getQString(L, 1);
  const optional<MiniWindow::Position> position =
    qlua::getEnum<MiniWindow::Position>(L, 2);
  expect_nonnull(position, ApiCode::BadParameter);
  return returnCode(L, getApi(L).SetForegroundImage(path, *position));
}

int
L_SetMainTitle(lua_State* L)
{
  BENCHMARK
  getApi(L).SetMainTitle(QString::fromUtf8(qlua::concatArgs(L)));
  return 0;
}

int
L_SetStatus(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  getApi(L).SetStatus(QString::fromUtf8(qlua::concatArgs(L)));
  return 0;
}

int
L_Simulate(lua_State* L)
{
  BENCHMARK
  const QByteArray message = qlua::concatArgs(L);
  getApi(L).Simulate(string_view(message.data(), message.size()));
  return 0;
}

int
L_Tell(lua_State* L)
{
  BENCHMARK
  getApi(L).Tell(QString::fromUtf8(qlua::concatArgs(L)));
  return 0;
}

// plugin

int
L_BroadcastPlugin(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  push(L,
       getApi(L).BroadcastPlugin(
         getPluginIndex(L), qlua::getInteger(L, 1), qlua::getString(L, 2)));
  return 1;
}

int
L_CallPlugin(lua_State* L)
{
  BENCHMARK
  const Plugin* pluginRef = getApi(L).getPlugin(qlua::getString(L, 1));
  if (pluginRef == nullptr) [[unlikely]] {
    return returnCode(
      L, ApiCode::NoSuchPlugin, fmtNoSuchPlugin(qlua::getString(L, 1)));
  }

  const Plugin& plugin = *pluginRef;
  if (plugin.isDisabled()) [[unlikely]] {
    return returnCode(L, ApiCode::PluginDisabled, fmtPluginDisabled(plugin));
  }

  const string_view routine = qlua::getString(L, 2);

  const ScriptThread thread = plugin.spawnThread();
  lua_State* L2 = thread.state();

  const int nargs = lua_gettop(L) - 2;
  luaL_checkstack(L2, nargs + 1, nullptr);

  const char* data = routine.data();
  if (lua_getglobal(L2, data) != LUA_TFUNCTION) {
    lua_pop(L2, 1);
    return returnCode(
      L, ApiCode::NoSuchRoutine, fmtNoSuchRoutine(plugin, routine));
  }

  const int topBefore = lua_gettop(L2) - 1;

  for (int i = 1; i <= nargs; ++i) {
    if (!qlua::copyValue(L, L2, i + 2)) [[unlikely]] {
      lua_settop(L, 0);
      return returnCode(
        L, ApiCode::BadParameter, fmtBadParam(i - 2, luaL_typename(L, i)));
    }
  }

  if (!api_pcall(L2, nargs, LUA_MULTRET)) {
    lua_settop(L, 0);
    push(L, ApiCode::ErrorCallingPluginRoutine);
    push(L, fmtCallError(plugin, routine));
    size_t size = 0;
    lua_pushlstring(L, lua_tolstring(L2, -1, &size), size);
    return 3;
  }

  const int topAfter = lua_gettop(L2);
  const int nresults = topAfter - topBefore;
  lua_settop(L, 0);
  luaL_checkstack(L, nresults + 1, nullptr);
  push(L, ApiCode::OK);
  for (int i = topBefore + 1; i <= topAfter; ++i) {
    if (!qlua::copyValue(L2, L, i)) [[unlikely]] {
      return returnCode(L,
                        ApiCode::ErrorCallingPluginRoutine,
                        fmtBadReturn(plugin, routine, i, luaL_typename(L2, i)));
    }
  }
  return nresults + 1;
}

int
L_EnablePlugin(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  return returnCode(
    L,
    getApi(L).EnablePlugin(qlua::getString(L, 1), qlua::getBool(L, 2, true)));
}

int
L_GetPluginID(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  push(L, getApi(L).GetPluginID(getPluginIndex(L)));
  return 1;
}

int
L_PluginSupports(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  return returnCode(
    L, getApi(L).PluginSupports(qlua::getString(L, 1), qlua::getString(L, 2)));
}

// sender

int
L_AddAlias(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 5);
  const string_view name = qlua::getString(L, 1);
  const string_view pattern = qlua::getString(L, 2);
  const string_view text = qlua::getString(L, 3);
  const AliasFlags flags = qlua::getQFlags<AliasFlag>(L, 4);
  const optional<string_view> script = qlua::getScriptName(L, 5);
  expect_nonnull(script, ApiCode::ScriptNameNotLocated);

  return returnCode(
    L,
    getApi(L).AddAlias(getPluginIndex(L), name, pattern, text, flags, *script));
}

int
L_AddTimer(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 7);
  const string_view name = qlua::getString(L, 1);
  const int hour = qlua::getInt(L, 2);
  const int minute = qlua::getInt(L, 3);
  const lua_Number second = qlua::getNumber(L, 4);
  const string_view text = qlua::getString(L, 5);
  const TimerFlags flags = qlua::getQFlags<TimerFlag>(L, 6);
  const optional<string_view> script = qlua::getScriptName(L, 7);
  expect_nonnull(script, ApiCode::ScriptNameNotLocated);

  return returnCode(
    L,
    getApi(L).AddTimer(
      getPluginIndex(L), name, hour, minute, second, text, flags, *script));
}

int
L_AddTrigger(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 10);
  const string_view name = qlua::getString(L, 1);
  const string_view pattern = qlua::getString(L, 2);
  const string_view text = qlua::getString(L, 3);
  const TriggerFlags flags = qlua::getQFlags<TriggerFlag>(L, 4);
  const QColor color = qlua::getCustomColor(L, 5);
  // const lua_Integer wildcardIndex = qlua::getInt(L, 6);
  const string_view soundFile = qlua::getString(L, 7);
  const optional<string_view> script = qlua::getScriptName(L, 8);
  const optional<SendTarget> target = qlua::getEnum(L, 9, SendTarget::World);
  const int sequence = qlua::getInt(L, 10, 100);

  expect_nonnull(script, ApiCode::ScriptNameNotLocated);
  expect_nonnull(target, ApiCode::TriggerSendToInvalid);

  return returnCode(L,
                    getApi(L).AddTrigger(getPluginIndex(L),
                                         name,
                                         pattern,
                                         text,
                                         flags,
                                         color,
                                         soundFile,
                                         *script,
                                         *target,
                                         sequence));
}

int
L_DeleteAlias(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  return returnCode(
    L, getApi(L).DeleteAlias(getPluginIndex(L), qlua::getString(L, 1)));
}

int
L_DeleteAliasGroup(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, getApi(L).DeleteAliasGroup(getPluginIndex(L), qlua::getString(L, 1)));
  return 1;
}

int
L_DeleteGroup(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  const string_view group = qlua::getString(L, 1);
  const size_t pluginIndex = getPluginIndex(L);
  ScriptApi& api = getApi(L);
  push(L,
       api.DeleteAliasGroup(pluginIndex, group) +
         api.DeleteTimerGroup(pluginIndex, group) +
         api.DeleteTriggerGroup(pluginIndex, group));
  return 1;
}

int
L_DeleteTemporaryAliases(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  push(L, getApi(L).DeleteTemporaryAliases());
  return 1;
}

int
L_DeleteTemporaryTimers(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  push(L, getApi(L).DeleteTemporaryTimers());
  return 1;
}

int
L_DeleteTemporaryTriggers(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  push(L, getApi(L).DeleteTemporaryTriggers());
  return 1;
}

int
L_DeleteTimer(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  return returnCode(
    L, getApi(L).DeleteTimer(getPluginIndex(L), qlua::getString(L, 1)));
}

int
L_DeleteTimerGroup(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, getApi(L).DeleteTimerGroup(getPluginIndex(L), qlua::getString(L, 1)));
  return 1;
}

int
L_DeleteTrigger(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  return returnCode(
    L, getApi(L).DeleteTrigger(getPluginIndex(L), qlua::getString(L, 1)));
}

int
L_DeleteTriggerGroup(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L,
       getApi(L).DeleteTriggerGroup(getPluginIndex(L), qlua::getString(L, 1)));
  return 1;
}

int
L_DoAfter(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  const lua_Number seconds = qlua::getNumber(L, 1);
  const QString text = qlua::getQString(L, 2);
  return returnCode(
    L,
    getApi(L).DoAfter(getPluginIndex(L), seconds, text, SendTarget::Command));
}

int
L_DoAfterNote(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  const lua_Number seconds = qlua::getNumber(L, 1);
  const QString text = qlua::getQString(L, 2);
  return returnCode(
    L, getApi(L).DoAfter(getPluginIndex(L), seconds, text, SendTarget::Output));
}

int
L_DoAfterSpecial(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 3);
  const lua_Number seconds = qlua::getNumber(L, 1);
  const QString text = qlua::getQString(L, 2);
  const optional<SendTarget> target = qlua::getEnum<SendTarget>(L, 3);
  expect_nonnull(target, ApiCode::OptionOutOfRange);
  return returnCode(
    L, getApi(L).DoAfter(getPluginIndex(L), seconds, text, *target));
}

int
L_DoAfterSpeedwalk(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  const lua_Number seconds = qlua::getNumber(L, 1);
  const QString text = qlua::getQString(L, 2);
  return returnCode(
    L,
    getApi(L).DoAfter(getPluginIndex(L), seconds, text, SendTarget::Speedwalk));
}

int
L_EnableAlias(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  return returnCode(L,
                    getApi(L).EnableAlias(getPluginIndex(L),
                                          qlua::getString(L, 1),
                                          qlua::getBool(L, 2, true)));
}

int
L_EnableAliasGroup(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  push(L,
       getApi(L).EnableAliasGroup(
         getPluginIndex(L), qlua::getString(L, 1), qlua::getBool(L, 2, true)));
  return 1;
}

int
L_EnableGroup(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  const size_t pluginIndex = getPluginIndex(L);
  const string_view group = qlua::getString(L, 1);
  const bool enable = qlua::getBool(L, 2, true);
  ScriptApi& api = getApi(L);
  push(L,
       api.EnableAliasGroup(pluginIndex, group, enable) +
         api.EnableTimerGroup(pluginIndex, group, enable) +
         api.EnableTriggerGroup(pluginIndex, group, enable));
  return 1;
}

int
L_EnableTimer(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  return returnCode(L,
                    getApi(L).EnableTimer(getPluginIndex(L),
                                          qlua::getString(L, 1),
                                          qlua::getBool(L, 2, true)));
}

int
L_EnableTimerGroup(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  push(L,
       getApi(L).EnableTimerGroup(
         getPluginIndex(L), qlua::getString(L, 1), qlua::getBool(L, 2, true)));
  return 1;
}

int
L_EnableTrigger(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  return returnCode(L,
                    getApi(L).EnableTrigger(getPluginIndex(L),
                                            qlua::getString(L, 1),
                                            qlua::getBool(L, 2, true)));
}

int
L_EnableTriggerGroup(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  push(L,
       getApi(L).EnableTriggerGroup(
         getPluginIndex(L), qlua::getString(L, 1), qlua::getBool(L, 2, true)));
  return 1;
}

int
L_ExportXML(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  const optional<ExportKind> kind = qlua::getEnum<ExportKind>(L, 1);
  const string_view name = qlua::getString(L, 2);
  if (!kind) {
    push(L, "");
    return 1;
  }
  push(L, getApi(L).ExportXML(getPluginIndex(L), *kind, name));
  return 1;
}

int
L_GetAliasOption(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  const size_t plugin = getPluginIndex(L);
  const string_view label = qlua::getString(L, 1);
  const string_view option = qlua::getString(L, 2);
  pushQVariant(L, getApi(L).GetAliasOption(plugin, label, option));
  return 1;
}

int
L_GetTimerOption(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  const size_t plugin = getPluginIndex(L);
  const string_view label = qlua::getString(L, 1);
  const string_view option = qlua::getString(L, 2);
  pushQVariant(L, getApi(L).GetTimerOption(plugin, label, option));
  return 1;
}

int
L_GetTriggerOption(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  const size_t plugin = getPluginIndex(L);
  const string_view label = qlua::getString(L, 1);
  const string_view option = qlua::getString(L, 2);
  pushQVariant(L, getApi(L).GetTriggerOption(plugin, label, option));
  return 1;
}

int
L_IsAlias(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  return returnCode(
    L, getApi(L).IsAlias(getPluginIndex(L), qlua::getString(L, 1)));
}

int
L_IsTimer(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  return returnCode(
    L, getApi(L).IsTimer(getPluginIndex(L), qlua::getString(L, 1)));
}

int
L_IsTrigger(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  return returnCode(
    L, getApi(L).IsTrigger(getPluginIndex(L), qlua::getString(L, 1)));
}

int
L_MakeRegularExpression(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, ScriptApi::MakeRegularExpression(qlua::getString(L, 1)));
  return 1;
}

int
L_SetAliasOption(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 3);
  const size_t plugin = getPluginIndex(L);
  const string_view label = qlua::getString(L, 1);
  const string_view option = qlua::getString(L, 2);
  const optional<string_view> value = getSenderOption(L, 3);
  expect_nonnull(value, ApiCode::OptionOutOfRange);
  return returnCode(L, getApi(L).SetAliasOption(plugin, label, option, *value));
}

int
L_SetTimerOption(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 3);
  const size_t plugin = getPluginIndex(L);
  const string_view label = qlua::getString(L, 1);
  const string_view option = qlua::getString(L, 2);
  const optional<string_view> value = getSenderOption(L, 3);
  expect_nonnull(value, ApiCode::OptionOutOfRange);
  return returnCode(L, getApi(L).SetTimerOption(plugin, label, option, *value));
}

int
L_SetTitle(lua_State* L)
{
  BENCHMARK
  getApi(L).SetTitle(QString::fromUtf8(qlua::concatArgs(L)));
  return 0;
}

int
L_SetTriggerOption(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 3);
  const size_t plugin = getPluginIndex(L);
  const string_view label = qlua::getString(L, 1);
  const string_view option = qlua::getString(L, 2);
  const optional<string_view> value = getSenderOption(L, 3);
  expect_nonnull(value, ApiCode::OptionOutOfRange);
  return returnCode(L,
                    getApi(L).SetTriggerOption(plugin, label, option, *value));
}

int
L_StopEvaluatingTriggers(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  getApi(L).StopEvaluatingTriggers();
  return 0;
}

// sound

int
L_PlaySound(lua_State* L)
{
  BENCHMARK
  // lua_Integer PlaySound(short Buffer, BSTR FileName, BOOL Loop, double
  // Volume, double Pan);
  expectMaxArgs(L, 5);
  return returnCode(
    L,
    getApi(L).PlaySound(qlua::getInteger(L, 1),
                        qlua::getString(L, 2),
                        qlua::getBool(L, 3, false),
                        convertVolume(qlua::getNumber(L, 4, 0.0))));
  // qlua::getDouble(L, 5) pan
}

int
L_PlaySoundMemory(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 5);
  return returnCode(
    L,
    getApi(L).PlaySoundMemory(qlua::getInteger(L, 1),
                              qlua::getBytes(L, 2),
                              qlua::getBool(L, 3, false),
                              convertVolume(qlua::getNumber(L, 4, 0.0))));
  // qlua::getDouble(L, 5) pan
}

int
L_StopSound(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  return returnCode(L, getApi(L).StopSound(qlua::getInteger(L, 1, 0)));
}

// variable

int
L_DeleteVariable(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  return returnCode(
    L, getApi(L).DeleteVariable(getPluginIndex(L), qlua::getString(L, 1)));
}

int
L_GetVariable(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  pushVariable(L,
               getApi(L).GetVariable(getPluginIndex(L), qlua::getString(L, 1)));
  return 1;
}

int
L_GetPluginVariable(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  pushVariable(
    L, getApi(L).GetVariable(qlua::getString(L, 1), qlua::getString(L, 2)));
  return 1;
}

int
L_SetVariable(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  getApi(L).SetVariable(
    getPluginIndex(L), qlua::getString(L, 1), qlua::getString(L, 2));
  return returnCode(L, ApiCode::OK);
}

// window

int
L_TextRectangle(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 9);
  const QRect rect = qlua::getQRect(L, 1, 2, 3, 4);
  const int offset = qlua::getInt(L, 5);
  QColor borderColor = qlua::getQColor(L, 6);
  const int borderWidth = qlua::getInt(L, 7);
  QColor outsideColor = qlua::getQColor(L, 8);
  const optional<Qt::BrushStyle> outsideFillStyle = qlua::getBrush(L, 9);
  expect_nonnull(outsideFillStyle, ApiCode::BrushStyleNotValid);
  if (borderColor == Qt::GlobalColor::black) {
    borderColor = Qt::GlobalColor::transparent;
  }
  if (outsideColor == Qt::GlobalColor::black) {
    outsideColor = Qt::GlobalColor::transparent;
  }
  return returnCode(
    L,
    getApi(L).TextRectangle(rect,
                            offset,
                            borderColor,
                            borderWidth,
                            QBrush(outsideColor, *outsideFillStyle)));
}

int
L_WindowArc(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 12);
  const string_view windowName = qlua::getString(L, 1);
  const QRectF rect = qlua::getQRectF(L, 2, 3, 4, 5);
  const QPointF start = qlua::getQPointF(L, 6, 7);
  const QPointF end = qlua::getQPointF(L, 8, 9);
  const optional<QPen> pen = qlua::getQPen(L, 10, 11, 12);
  expect_nonnull(pen, ApiCode::PenStyleNotValid);
  return returnCode(L, getApi(L).WindowArc(windowName, rect, start, end, *pen));
}

int
L_WindowBlendImage(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 12);
  const string_view windowName = qlua::getString(L, 1);
  const string_view imageId = qlua::getString(L, 2);
  const QRectF rect = qlua::getQRectF(L, 3, 4, 5, 6);
  const optional<BlendMode> mode = qlua::getEnum<BlendMode>(L, 7);
  const lua_Number opacity = qlua::getNumber(L, 8);
  const QRectF targetRect = qlua::getQRectF(L, 9, 10, 11, 12);
  expect_nonnull(mode, ApiCode::UnknownOption);
  if (opacity < 0 || opacity > 1) {
    return returnCode(L, ApiCode::BadParameter);
  }
  return returnCode(L,
                    getApi(L).WindowBlendImage(
                      windowName, imageId, rect, *mode, opacity, targetRect));
}

int
L_WindowCircleOp(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 15);
  const string_view windowName = qlua::getString(L, 1);
  const optional<CircleOp> action = qlua::getEnum<CircleOp>(L, 2);
  const QRectF rect = qlua::getQRectF(L, 3, 4, 5, 6);
  const optional<QPen> pen = qlua::getQPen(L, 7, 8, 9);
  const QColor brushColor = qlua::getQColor(L, 10);
  const optional<Qt::BrushStyle> brushStyle =
    qlua::getBrush(L, 11, Qt::BrushStyle::SolidPattern);
  expect_nonnull(action, ApiCode::UnknownOption);
  expect_nonnull(pen, ApiCode::PenStyleNotValid);
  expect_nonnull(brushStyle, ApiCode::BrushStyleNotValid);
  const QBrush brush(brushColor, *brushStyle);

  switch (*action) {
    case CircleOp::Ellipse:
      return returnCode(L,
                        getApi(L).WindowEllipse(windowName, rect, *pen, brush));
    case CircleOp::Rectangle:
      return returnCode(L, getApi(L).WindowRect(windowName, rect, *pen, brush));
    case CircleOp::RoundedRectangle:
      return returnCode(L,
                        getApi(L).WindowRoundedRect(windowName,
                                                    rect,
                                                    qlua::getNumber(L, 12, 0),
                                                    qlua::getNumber(L, 13, 0),
                                                    *pen,
                                                    brush));
    case CircleOp::Chord:
    case CircleOp::Pie:
      return returnCode(L, ApiCode::OK);
  }
}

int
L_WindowCreate(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 8);
  const string_view windowName = qlua::getString(L, 1);
  const QPoint location = qlua::getQPoint(L, 2, 3);
  const QSize size = qlua::getQSize(L, 4, 5);
  const optional<MiniWindow::Position> position =
    qlua::getEnum<MiniWindow::Position>(L, 6);
  const MiniWindow::Flags flags = qlua::getQFlags<MiniWindow::Flag>(L, 7);
  const QColor bg = qlua::getQColor(L, 8);
  expect_nonnull(position, ApiCode::BadParameter);
  return returnCode(
    L,
    getApi(L).WindowCreate(
      getPluginIndex(L), windowName, location, size, *position, flags, bg));
}

int
L_WindowCreateImage(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 10);
  return returnCode(L,
                    getApi(L).WindowCreateImage(qlua::getString(L, 1),
                                                qlua::getString(L, 2),
                                                { qlua::getInteger(L, 10),
                                                  qlua::getInteger(L, 9),
                                                  qlua::getInteger(L, 8),
                                                  qlua::getInteger(L, 7),
                                                  qlua::getInteger(L, 6),
                                                  qlua::getInteger(L, 5),
                                                  qlua::getInteger(L, 4),
                                                  qlua::getInteger(L, 3) }));
}

int
L_WindowDelete(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  return returnCode(L, getApi(L).WindowDelete(qlua::getString(L, 1)));
}

int
L_WindowDrawImage(lua_State* L)
{
  BENCHMARK
  const int n = expectMaxArgs(L, 11);
  const string_view windowName = qlua::getString(L, 1);
  const string_view imageID = qlua::getString(L, 2);
  const QRectF rect = qlua::getQRectF(L, 3, 4, 5, 6);
  const optional<DrawImageMode> mode = qlua::getEnum(L, 7, DrawImageMode::Copy);
  const QRectF sourceRect =
    n >= 8 ? qlua::getQRectF(L, 8, 9, 10, 11) : QRectF();
  expect_nonnull(mode, ApiCode::BadParameter);
  return returnCode(
    L, getApi(L).WindowDrawImage(windowName, imageID, rect, *mode, sourceRect));
}

int
L_WindowDrawImageAlpha(lua_State* L)
{
  BENCHMARK
  const int n = expectMaxArgs(L, 9);
  const string_view windowName = qlua::getString(L, 1);
  const string_view imageID = qlua::getString(L, 2);
  const QRectF rect = qlua::getQRectF(L, 3, 4, 5, 6);
  const lua_Number opacity = qlua::getNumber(L, 7);
  const QPointF origin = n >= 8 ? qlua::getQPointF(L, 8, 9) : QPointF();
  if (opacity < 0 || opacity > 1) {
    return returnCode(L, ApiCode::BadParameter);
  }
  return returnCode(
    L,
    getApi(L).WindowDrawImageAlpha(windowName, imageID, rect, opacity, origin));
}

struct FilterParams
{
  lua_State* L;
  string_view windowName;
  QRect rect;

  int filter(const ImageFilter& filter) const
  {
    return returnCode(L, getApi(L).WindowFilter(windowName, filter, rect));
  }

  template<typename T>
  int convolve() const
  {
    const optional<ImageFilter::Directions> directions =
      qlua::getEnum(L, 7, ImageFilter::Directions::Both);
    expect_nonnull(directions, ApiCode::BadParameter);
    return filter(T(*directions));
  }
};

int
L_WindowFilter(lua_State* L)
{
  BENCHMARK
  using ImageFilter::ColorChannel::Blue;
  using ImageFilter::ColorChannel::Green;
  using ImageFilter::ColorChannel::Red;
  expectMaxArgs(L, 7);
  const string_view windowName = qlua::getString(L, 1);
  const QRect rect = qlua::getQRect(L, 2, 3, 4, 5);
  const FilterParams params{ .L = L, .windowName = windowName, .rect = rect };
  const optional<FilterOp> filterOp = qlua::getEnum<FilterOp>(L, 6);
  expect_nonnull(filterOp, ApiCode::UnknownOption);
  switch (*filterOp) {
    case FilterOp::Noise:
      return params.filter(ImageFilter::Noise(qlua::getNumber(L, 7)));
    case FilterOp::MonoNoise:
      return params.filter(ImageFilter::MonoNoise(qlua::getNumber(L, 7)));
    case FilterOp::Blur:
      return params.convolve<ImageFilter::Blur>();
    case FilterOp::Sharpen:
      return params.convolve<ImageFilter::Sharpen>();
    case FilterOp::EdgeDetect:
      return params.convolve<ImageFilter::EdgeDetect>();
    case FilterOp::Emboss:
      return params.convolve<ImageFilter::Emboss>();
    case FilterOp::BrightnessAdd:
      return params.filter(ImageFilter::BrightnessAdd(qlua::getInt(L, 7)));
    case FilterOp::Contrast:
      return params.filter(ImageFilter::Contrast(qlua::getNumber(L, 7)));
    case FilterOp::Gamma:
      return params.filter(ImageFilter::Gamma(qlua::getNumber(L, 7)));
    case FilterOp::RedBrightnessAdd:
      return params.filter(ImageFilter::BrightnessAdd(qlua::getInt(L, 7), Red));
    case FilterOp::RedContrast:
      return params.filter(ImageFilter::Contrast(qlua::getNumber(L, 7), Red));
    case FilterOp::RedGamma:
      return params.filter(ImageFilter::Gamma(qlua::getNumber(L, 7), Red));
    case FilterOp::GreenBrightnessAdd:
      return params.filter(
        ImageFilter::BrightnessAdd(qlua::getInt(L, 7), Green));
    case FilterOp::GreenContrast:
      return params.filter(ImageFilter::Contrast(qlua::getNumber(L, 7), Green));
    case FilterOp::GreenGamma:
      return params.filter(ImageFilter::Gamma(qlua::getNumber(L, 7), Green));
    case FilterOp::BlueBrightnessAdd:
      return params.filter(
        ImageFilter::BrightnessAdd(qlua::getInt(L, 7), Blue));
    case FilterOp::BlueContrast:
      return params.filter(ImageFilter::Contrast(qlua::getNumber(L, 7), Blue));
    case FilterOp::BlueGamma:
      return params.filter(ImageFilter::Gamma(qlua::getNumber(L, 7), Blue));
    case FilterOp::GrayscaleLinear:
      return params.filter(ImageFilter::GrayscaleLinear());
    case FilterOp::GrayscalePerceptual:
      return params.filter(ImageFilter::GrayscalePerceptual());
    case FilterOp::BrightnessMult:
      return params.filter(ImageFilter::BrightnessMult(qlua::getNumber(L, 7)));
    case FilterOp::RedBrightnessMult:
      return params.filter(
        ImageFilter::BrightnessMult(qlua::getNumber(L, 7), Red));
    case FilterOp::GreenBrightnessMult:
      return params.filter(
        ImageFilter::BrightnessMult(qlua::getNumber(L, 7), Green));
    case FilterOp::BlueBrightnessMult:
      return params.filter(
        ImageFilter::BrightnessMult(qlua::getNumber(L, 7), Blue));
    case FilterOp::LesserBlur:
      return params.convolve<ImageFilter::LesserBlur>();
    case FilterOp::MinorBlur:
      return params.convolve<ImageFilter::MinorBlur>();
    case FilterOp::Average:
      return params.filter(ImageFilter::Average());
  }
}

int
L_WindowFont(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 10);
  const string_view windowName = qlua::getString(L, 1);
  const string_view fontID = qlua::getString(L, 2);
  const QString fontName = qlua::getQString(L, 3);
  const lua_Number pointSize = qlua::getNumber(L, 4);
  if (pointSize == 0 && fontName.isEmpty()) [[unlikely]] {
    return returnCode(L, getApi(L).WindowUnloadFont(windowName, fontID));
  }
  const bool bold = qlua::getBool(L, 5, false);
  const bool italic = qlua::getBool(L, 6, false);
  const bool underline = qlua::getBool(L, 7, false);
  const bool strikeout = qlua::getBool(L, 8, false);
  // const short charset = qlua::getInt(L, 9);
  const lua_Integer fontFlag = qlua::getInteger(L, 9, 0);
  const optional<ScriptFont> font = ScriptFont::validate(fontFlag);
  expect_nonnull(font, ApiCode::BadParameter);
  return returnCode(L,
                    getApi(L).WindowFont(windowName,
                                         fontID,
                                         fontName,
                                         pointSize,
                                         bold,
                                         italic,
                                         underline,
                                         strikeout,
                                         font->hint()));
}

int
L_WindowFontList(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  pushListOrEmpty(L, getApi(L).WindowFontList(qlua::getString(L, 1)));
  return 1;
}

int
L_WindowGetImageAlpha(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 8);
  return returnCode(
    L,
    getApi(L).WindowGetImageAlpha(qlua::getString(L, 1),
                                  qlua::getString(L, 2),
                                  qlua::getQRectF(L, 3, 4, 5, 6),
                                  qlua::getQPointF(L, 7, 8)));
}

int
L_WindowGetPixel(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 3);
  const optional<QColor> optColor =
    getApi(L).WindowGetPixel(qlua::getString(L, 1), qlua::getQPoint(L, 2, 3));
  if (optColor == nullopt) {
    push(L, -2);
    return 1;
  }
  const QColor color = *optColor;
  if (color.isValid()) {
    push(L, color);
  } else {
    push(L, -1);
  }
  return 1;
}

int
L_WindowGradient(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 8);
  const string_view windowName = qlua::getString(L, 1);
  const QRectF rect = qlua::getQRectF(L, 2, 3, 4, 5);
  const QColor color1 = qlua::getQColor(L, 6);
  const QColor color2 = qlua::getQColor(L, 7);
  const optional<Qt::Orientation> mode = qlua::getEnum<Qt::Orientation>(L, 8);
  expect_nonnull(mode, ApiCode::UnknownOption);
  return returnCode(
    L, getApi(L).WindowGradient(windowName, rect, color1, color2, *mode));
}

int
L_WindowImageFromWindow(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 3);
  return returnCode(L,
                    getApi(L).WindowImageFromWindow(qlua::getString(L, 1),
                                                    qlua::getString(L, 2),
                                                    qlua::getString(L, 3)));
}

int
L_WindowImageList(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  pushListOrEmpty(L, getApi(L).WindowImageList(qlua::getString(L, 1)));
  return 1;
}

int
L_WindowImageOp(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 13);
  const string_view windowName = qlua::getString(L, 1);
  const optional<ImageOp> action = qlua::getEnum<ImageOp>(L, 2);
  const QRectF rect = qlua::getQRectF(L, 3, 4, 5, 6);
  const optional<QPen> pen = qlua::getQPen(L, 7, 8, 9);
  const QColor color = qlua::getQColor(L, 10);
  const string_view imageID = qlua::getString(L, 11);
  expect_nonnull(action, ApiCode::UnknownOption);
  expect_nonnull(pen, ApiCode::PenStyleNotValid);
  switch (*action) {
    case ImageOp::Ellipse:
      return returnCode(
        L, getApi(L).WindowEllipse(windowName, rect, *pen, color, imageID));
    case ImageOp::Rectangle:
      return returnCode(
        L, getApi(L).WindowRect(windowName, rect, *pen, color, imageID));
    case ImageOp::RoundedRectangle:
      return returnCode(L,
                        getApi(L).WindowRoundedRect(windowName,
                                                    rect,
                                                    qlua::getNumber(L, 12),
                                                    qlua::getNumber(L, 13),
                                                    *pen,
                                                    color,
                                                    imageID));
  }
}

int
L_WindowLine(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 8);
  const string_view windowName = qlua::getString(L, 1);
  const QLineF line = qlua::getQLineF(L, 2, 3, 4, 5);
  const optional<QPen> pen = qlua::getQPen(L, 6, 7, 8);
  expect_nonnull(pen, ApiCode::PenStyleNotValid);
  return returnCode(L, getApi(L).WindowLine(windowName, line, *pen));
}

int
L_WindowList(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  pushList(L, getApi(L).WindowList());
  return 1;
}

int
L_WindowLoadImage(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 3);
  const string_view windowName = qlua::getString(L, 1);
  const string_view imageID = qlua::getString(L, 2);
  const QString filename = qlua::getQString(L, 3);
  if (filename.isEmpty()) [[unlikely]] {
    return returnCode(L, getApi(L).WindowUnloadImage(windowName, imageID));
  }
  return returnCode(L,
                    getApi(L).WindowLoadImage(windowName, imageID, filename));
}

int
L_WindowMenu(lua_State* L)
{
  expectMaxArgs(L, 4);
  pushQVariant(L,
               getApi(L).WindowMenu(qlua::getString(L, 1),
                                    qlua::getQPoint(L, 2, 3),
                                    qlua::getString(L, 4)));
  return 1;
}

int
L_WindowMergeImageAlpha(lua_State* L)
{
  expectMaxArgs(L, 13);
  const string_view windowName = qlua::getString(L, 1);
  const string_view imageID = qlua::getString(L, 2);
  const string_view maskID = qlua::getString(L, 3);
  const QRect targetRect = qlua::getQRect(L, 4, 5, 6, 7);
  const optional<MergeMode> mode = qlua::getEnum<MergeMode>(L, 8);
  const qreal opacity = qlua::getNumber(L, 9);
  const QRect sourceRect = qlua::getQRect(L, 10, 11, 12, 13);
  expect_nonnull(mode, ApiCode::UnknownOption);
  if (opacity < 0 || opacity > 1) {
    return returnCode(L, ApiCode::BadParameter);
  }
  return returnCode(
    L,
    getApi(L).WindowMergeImageAlpha(
      windowName, imageID, maskID, targetRect, *mode, opacity, sourceRect));
}

int
L_WindowPolygon(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 9);
  const string_view windowName = qlua::getString(L, 1);
  const optional<QPolygonF> polygon = qlua::getQPolygonF(L, 2);
  const optional<QPen> pen = qlua::getQPen(L, 3, 4, 5);
  const QColor brushColor = qlua::getQColor(L, 6);
  const optional<Qt::BrushStyle> brushStyle = qlua::getBrush(L, 7);
  const bool close = qlua::getBool(L, 8, false);
  const bool winding = qlua::getBool(L, 9, false);
  expect_nonnull(polygon, ApiCode::InvalidNumberOfPoints);
  expect_nonnull(pen, ApiCode::PenStyleNotValid);
  expect_nonnull(brushStyle, ApiCode::BrushStyleNotValid);
  return returnCode(L,
                    getApi(L).WindowPolygon(windowName,
                                            *polygon,
                                            *pen,
                                            QBrush(brushColor, *brushStyle),
                                            close,
                                            winding
                                              ? Qt::FillRule::WindingFill
                                              : Qt::FillRule::OddEvenFill));
}

int
L_WindowPosition(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 5);
  const string_view windowName = qlua::getString(L, 1);
  const QPoint location = qlua::getQPoint(L, 2, 3);
  const optional<MiniWindow::Position> position =
    qlua::getEnum<MiniWindow::Position>(L, 4);
  const MiniWindow::Flags flags = qlua::getQFlags<MiniWindow::Flag>(L, 5);
  expect_nonnull(position, ApiCode::BadParameter);
  return returnCode(
    L, getApi(L).WindowPosition(windowName, location, *position, flags));
}

int
L_WindowRectOp(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 8);
  const string_view windowName = qlua::getString(L, 1);
  const optional<RectOp> action = qlua::getEnum<RectOp>(L, 2);
  const QRectF rect = qlua::getQRectF(L, 3, 4, 5, 6);
  expect_nonnull(action, ApiCode::UnknownOption);
  switch (*action) {
    case RectOp::Frame:
      return returnCode(L,
                        getApi(L).WindowRect(
                          windowName, rect, qlua::getQColor(L, 7), QBrush()));
    case RectOp::Fill:
      return returnCode(
        L,
        getApi(L).WindowRect(windowName, rect, QPen(), qlua::getQColor(L, 7)));
    case RectOp::Invert:
      return returnCode(L, getApi(L).WindowInvert(windowName, rect.toRect()));
    case RectOp::Frame3D:
      return returnCode(
        L,
        getApi(L).WindowFrame(windowName,
                              rect,
                              qlua::getQColor(L, 7),
                              qlua::getQColor(L, 8, Qt::GlobalColor::black)));
    case RectOp::Edge3D:
      if (optional<ButtonFrame> frame = qlua::getEnum<ButtonFrame>(L, 7);
          frame) {
        return returnCode(L,
                          getApi(L).WindowButton(
                            windowName,
                            rect.toRect(),
                            *frame,
                            qlua::getQFlags<MiniWindow::ButtonFlag>(L, 8)));
      }
      return returnCode(L, ApiCode::BadParameter);
    case RectOp::FloodFillBorder:
    case RectOp::FloodFillSurface:
      return returnCode(L, ApiCode::OK);
  }
}

int
L_WindowResize(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 4);
  return returnCode(L,
                    getApi(L).WindowResize(qlua::getString(L, 1),
                                           qlua::getQSize(L, 2, 3),
                                           qlua::getQColor(L, 4)));
}

int
L_WindowSetPixel(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 3);
  return returnCode(L,
                    getApi(L).WindowSetPixel(qlua::getString(L, 1),
                                             qlua::getQPoint(L, 2, 3),
                                             qlua::getQColor(L, 4)));
}

int
L_WindowSetZOrder(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  return returnCode(
    L,
    getApi(L).WindowSetZOrder(qlua::getString(L, 1), qlua::getInteger(L, 2)));
}

int
L_WindowShow(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  return returnCode(
    L, getApi(L).WindowShow(qlua::getString(L, 1), qlua::getBool(L, 2, true)));
}

int
L_WindowText(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 9);
  const qreal width = getApi(L).WindowText(qlua::getString(L, 1),
                                           qlua::getString(L, 2),
                                           qlua::toString(L, 3),
                                           qlua::getQRectF(L, 4, 5, 6, 7),
                                           qlua::getQColor(L, 8),
                                           qlua::getBool(L, 9, false));
  push(L, static_cast<lua_Integer>(width));
  return 1;
}

int
L_WindowTextWidth(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 4);
  const int width = getApi(L).WindowTextWidth(qlua::getString(L, 1),
                                              qlua::getString(L, 2),
                                              qlua::getString(L, 3),
                                              qlua::getBool(L, 4, false));
  push(L, width);
  return 1;
}

int
L_WindowTransformImage(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 9);
  const string_view windowName = qlua::getString(L, 1);
  const string_view imageID = qlua::getString(L, 2);
  const lua_Integer modeN = qlua::getInteger(L, 5);
  const QTransform transform = qlua::getQTransform(L, 6, 7, 8, 9, 3, 4);
  MergeMode mode;
  switch (modeN) {
    case 1:
      mode = MergeMode::Straight;
      break;
    case 3:
      mode = MergeMode::Transparent;
      break;
    default:
      return returnCode(L, ApiCode::BadParameter);
  }
  return returnCode(
    L, getApi(L).WindowTransformImage(windowName, imageID, mode, transform));
}

int
L_WindowWrite(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  return returnCode(
    L, getApi(L).WindowWrite(qlua::getString(L, 1), qlua::getQString(L, 2)));
}

// window hotspot

int
L_WindowAddHotspot(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 14);
  const string_view windowName = qlua::getString(L, 1);
  const string_view hotspotID = qlua::getString(L, 2);
  const QRect geometry(qlua::getQPoint(L, 3, 4), qlua::getQPoint(L, 5, 6));
  Hotspot::Callbacks callbacks{
    .mouseOver = string(qlua::getString(L, 7, "")),
    .cancelMouseOver = string(qlua::getString(L, 8, "")),
    .mouseDown = string(qlua::getString(L, 9, "")),
    .cancelMouseDown = string(qlua::getString(L, 10, "")),
    .mouseUp = string(qlua::getString(L, 11, "")),
  };
  const QString tooltip = qlua::getQString(L, 12, QString());
  const optional<Qt::CursorShape> cursor =
    qlua::getCursor(L, 13, Qt::CursorShape::ArrowCursor);
  const Hotspot::Flags flags = qlua::getQFlags(L, 14, Hotspot::Flags());
  expect_nonnull(cursor, ApiCode::BadParameter);
  return returnCode(L,
                    getApi(L).WindowAddHotspot(getPluginIndex(L),
                                               windowName,
                                               hotspotID,
                                               geometry,
                                               std::move(callbacks),
                                               tooltip,
                                               *cursor,
                                               flags));
}

int
L_WindowDeleteAllHotspots(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  return returnCode(L,
                    getApi(L).WindowDeleteAllHotspots(qlua::getString(L, 1)));
}

int
L_WindowDeleteHotspot(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  return returnCode(L,
                    getApi(L).WindowDeleteHotspot(qlua::getString(L, 1),
                                                  qlua::getString(L, 2)));
}

int
L_WindowDragHandler(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 5);
  return returnCode(L,
                    getApi(L).WindowUpdateHotspot(
                      getPluginIndex(L),
                      qlua::getString(L, 1),
                      qlua::getString(L, 2),
                      Hotspot::CallbacksPartial{
                        .dragMove = string(qlua::getString(L, 3, "")),
                        .dragRelease = string(qlua::getString(L, 4, "")) }));
}

int
L_WindowHotspotList(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  pushListOrEmpty(L, getApi(L).WindowHotspotList(qlua::getString(L, 1)));
  return 1;
}

int
L_WindowHotspotTooltip(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 3);
  return returnCode(L,
                    getApi(L).WindowHotspotTooltip(qlua::getString(L, 1),
                                                   qlua::getString(L, 2),
                                                   qlua::getQString(L, 3)));
}

int
L_WindowMoveHotspot(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 6);
  return returnCode(
    L,
    getApi(L).WindowMoveHotspot(
      qlua::getString(L, 1),
      qlua::getString(L, 2),
      QRect(qlua::getQPoint(L, 3, 4), qlua::getQPoint(L, 5, 6))));
}

int
L_WindowScrollwheelHandler(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 3);
  return returnCode(L,
                    getApi(L).WindowUpdateHotspot(
                      getPluginIndex(L),
                      qlua::getString(L, 1),
                      qlua::getString(L, 2),
                      Hotspot::CallbacksPartial{
                        .scroll = string(qlua::getString(L, 3, "")) }));
}

// noop

#define NOOP(name, op)                                                         \
  int name(lua_State* L)                                                       \
  {                                                                            \
    op;                                                                        \
    return 1;                                                                  \
  }

NOOP(L_noop_echo, lua_pushvalue(L, 1));
NOOP(L_noop_empty, lua_createtable(L, 0, 0));
NOOP(L_noop_false, push(L, false));
NOOP(L_noop_neg, push(L, -1));
NOOP(L_noop_nil, lua_pushnil(L));
NOOP(L_noop_ok, push(L, ApiCode::OK));
NOOP(L_noop_spellcheck, push(L, ApiCode::SpellCheckNotActive));
NOOP(L_noop_string, lua_pushlstring(L, "", 0));

int
L_noop_void(lua_State* /* L */)
{
  return 0;
}

} // namespace

static const struct luaL_Reg worldlib[] =
  // color
  { { "AdjustColour", L_AdjustColour },
    { "ColourNameToRGB", L_ColourNameToRGB },
    { "GetBoldColour", L_GetBoldColour },
    { "GetCustomColourText", L_GetCustomColourText },
    { "GetMapColour", L_GetMapColour },
    { "GetNormalColour", L_GetNormalColour },
    { "GetSysColor", L_GetSysColor },
    { "MapColour", L_MapColour },
    { "MapColourList", L_MapColourList },
    { "NoteColourBack", L_NoteColourBack },
    { "NoteColourFore", L_NoteColourFore },
    { "NoteColourRgb", L_NoteColourRgb },
    { "NoteColourName", L_NoteColourRgb },
    { "PickColour", L_PickColour },
    { "RgbColourToName", L_RGBColourToName },
    { "SetBackgroundColour", L_SetBackgroundColour },
    { "SetBoldColour", L_SetBoldColour },
    { "SetNormalColour", L_SetNormalColour },
    // database
    { "DatabaseClose", L_DatabaseClose },
    { "DatabaseOpen", L_DatabaseOpen },
    // info
    { "GetInfo", L_GetInfo },
    { "GetLineInfo", L_GetLineInfo },
    { "GetPluginInfo", L_GetPluginInfo },
    { "GetStyleInfo", L_GetStyleInfo },
    { "GetTimerInfo", L_GetTimerInfo },
    { "GetUniqueNumber", L_GetUniqueNumber },
    { "Version", L_Version },
    { "WindowFontInfo", L_WindowFontInfo },
    { "WindowImageInfo", L_WindowImageInfo },
    { "WindowInfo", L_WindowInfo },
    // input
    { "Execute", L_Execute },
    { "LogSend", L_LogSend },
    { "Send", L_Send },
    { "SendImmediate", L_SendImmediate },
    { "SendNoEcho", L_SendNoEcho },
    { "SendPkt", L_SendPkt },
    { "SendPush", L_SendPush },
    // log
    { "CloseLog", L_CloseLog },
    { "FlushLog", L_FlushLog },
    { "IsLogOpen", L_IsLogOpen },
    { "OpenLog", L_OpenLog },
    { "WriteLog", L_WriteLog },
    // notepad
    { "ActivateNotepad", L_ActivateNotepad },
    { "AppendToNotepad", L_AppendToNotepad },
    { "CloseNotepad", L_CloseNotepad },
    { "GetNotepadLength", L_GetNotepadLength },
    { "GetNotepadList", L_GetNotepadList },
    { "GetNotepadText", L_GetNotepadText },
    { "GetNotepadWindowPosition", L_GetNotepadWindowPosition },
    { "NotepadColour", L_NotepadColour },
    { "NotepadFont", L_NotepadFont },
    { "NotepadReadOnly", L_NotepadReadOnly },
    { "NotepadSaveMethod", L_NotepadSaveMethod },
    { "ReplaceNotepad", L_ReplaceNotepad },
    { "SendToNotepad", L_SendToNotepad },
    // option
    { "GetAlphaOption", L_GetAlphaOption },
    { "GetAlphaOptionList", L_GetAlphaOptionList },
    { "GetCurrentValue", L_GetCurrentValue },
    { "GetOption", L_GetOption },
    { "GetOptionList", L_GetOptionList },
    { "SetAlphaOption", L_SetAlphaOption },
    { "SetOption", L_SetOption },
    // output
    { "Activate", L_ActivateClient },
    { "ActivateClient", L_ActivateClient },
    { "AddFont", L_AddFont },
    { "AnsiNote", L_AnsiNote },
    { "ColourNote", L_ColourNote },
    { "ColourTell", L_ColourTell },
    { "GetLinesInBufferCount", L_GetLinesInBufferCount },
    { "GetSysColor", L_GetSysColor },
    { "Hyperlink", L_Hyperlink },
    { "Note", L_Note },
    { "SetBackgroundImage", L_SetBackgroundImage },
    { "SetClipboard", L_SetClipboard },
    { "SetCursor", L_SetCursor },
    { "SetForegroundImage", L_SetForegroundImage },
    { "SetMainTitle", L_SetMainTitle },
    { "SetStatus", L_SetStatus },
    { "SetTitle", L_SetTitle },
    { "Simulate", L_Simulate },
    { "Tell", L_Tell },
    // plugin
    { "BroadcastPlugin", L_BroadcastPlugin },
    { "CallPlugin", L_CallPlugin },
    { "EnablePlugin", L_EnablePlugin },
    { "GetPluginID", L_GetPluginID },
    { "PluginSupports", L_PluginSupports },
    // sender
    { "AddAlias", L_AddAlias },
    { "AddTimer", L_AddTimer },
    { "AddTrigger", L_AddTrigger },
    { "AddTriggerEx", L_AddTrigger },
    { "DeleteAlias", L_DeleteAlias },
    { "DeleteAliasGroup", L_DeleteAliasGroup },
    { "DeleteGroup", L_DeleteGroup },
    { "DeleteTemporaryAliases", L_DeleteTemporaryAliases },
    { "DeleteTemporaryTimers", L_DeleteTemporaryTimers },
    { "DeleteTemporaryTriggers", L_DeleteTemporaryTriggers },
    { "DeleteTimer", L_DeleteTimer },
    { "DeleteTimerGroup", L_DeleteTimerGroup },
    { "DeleteTrigger", L_DeleteTrigger },
    { "DeleteTriggerGroup", L_DeleteTriggerGroup },
    { "DoAfter", L_DoAfter },
    { "DoAfterNote", L_DoAfterNote },
    { "DoAfterSpeedwalk", L_DoAfterSpeedwalk },
    { "DoAfterSpecial", L_DoAfterSpecial },
    { "EnableAlias", L_EnableAlias },
    { "EnableAliasGroup", L_EnableAliasGroup },
    { "EnableGroup", L_EnableGroup },
    { "EnableTimer", L_EnableTimer },
    { "EnableTimerGroup", L_EnableTimerGroup },
    { "EnableTrigger", L_EnableTrigger },
    { "EnableTriggerGroup", L_EnableTriggerGroup },
    { "ExportXML", L_ExportXML },
    { "GetAliasOption", L_GetAliasOption },
    { "GetTimerOption", L_GetTimerOption },
    { "GetTriggerOption", L_GetTriggerOption },
    { "IsAlias", L_IsAlias },
    { "IsTimer", L_IsTimer },
    { "IsTrigger", L_IsTrigger },
    { "MakeRegularExpression", L_MakeRegularExpression },
    { "SetAliasOption", L_SetAliasOption },
    { "SetTimerOption", L_SetTimerOption },
    { "SetTriggerOption", L_SetTriggerOption },
    { "StopEvaluatingTriggers", L_StopEvaluatingTriggers },
    // sound
    { "PlaySound", L_PlaySound },
    { "PlaySoundMemory", L_PlaySoundMemory },
    { "StopSound", L_StopSound },
    // variable
    { "DeleteVariable", L_DeleteVariable },
    { "GetVariable", L_GetVariable },
    { "GetPluginVariable", L_GetPluginVariable },
    { "SetVariable", L_SetVariable },
    // window
    { "TextRectangle", L_TextRectangle },
    { "WindowArc", L_WindowArc },
    { "WindowBlendImage", L_WindowBlendImage },
    { "WindowCircleOp", L_WindowCircleOp },
    { "WindowCreate", L_WindowCreate },
    { "WindowCreateImage", L_WindowCreateImage },
    { "WindowDelete", L_WindowDelete },
    { "WindowDrawImage", L_WindowDrawImage },
    { "WindowDrawImageAlpha", L_WindowDrawImageAlpha },
    { "WindowFilter", L_WindowFilter },
    { "WindowFont", L_WindowFont },
    { "WindowFontList", L_WindowFontList },
    { "WindowGetImageAlpha", L_WindowGetImageAlpha },
    { "WindowGetPixel", L_WindowGetPixel },
    { "WindowGradient", L_WindowGradient },
    { "WindowMergeImageAlpha", L_WindowMergeImageAlpha },
    { "WindowImageFromWindow", L_WindowImageFromWindow },
    { "WindowLine", L_WindowLine },
    { "WindowImageList", L_WindowImageList },
    { "WindowImageOp", L_WindowImageOp },
    { "WindowList", L_WindowList },
    { "WindowLoadImage", L_WindowLoadImage },
    { "WindowMenu", L_WindowMenu },
    { "WindowPolygon", L_WindowPolygon },
    { "WindowPosition", L_WindowPosition },
    { "WindowRectOp", L_WindowRectOp },
    { "WindowResize", L_WindowResize },
    { "WindowSetPixel", L_WindowSetPixel },
    { "WindowSetZOrder", L_WindowSetZOrder },
    { "WindowShow", L_WindowShow },
    { "WindowText", L_WindowText },
    { "WindowTextWidth", L_WindowTextWidth },
    { "WindowTransformImage", L_WindowTransformImage },
    { "Windowrite", L_WindowWrite },
    // window hotspot
    { "WindowAddHotspot", L_WindowAddHotspot },
    { "WindowDeleteAllHotspots", L_WindowDeleteAllHotspots },
    { "WindowDeleteHotspot", L_WindowDeleteHotspot },
    { "WindowDragHandler", L_WindowDragHandler },
    { "WindowHotspotList", L_WindowHotspotList },
    { "WindowHotspotTooltip", L_WindowHotspotTooltip },
    { "WindowMoveHotspot", L_WindowMoveHotspot },
    { "WindowScrollwheelHandler", L_WindowScrollwheelHandler },
    // stubs
    { "Accelerator", L_noop_ok },
    { "AcceleratorList", L_noop_empty },
    { "AcceleratorTo", L_noop_ok },
    { "AddMapperComment", L_noop_ok },
    { "AddSpellCheckWord", L_noop_spellcheck },
    { "AddToMapper", L_noop_ok },
    { "BlendPixel", L_noop_echo },
    { "Bookmark", L_noop_void },
    { "Debug", L_noop_nil },
    { "DeleteAllMapItems", L_noop_ok },
    { "DeleteLastMapItem", L_noop_ok },
    { "DiscardQueue", L_noop_ok },
    { "DoCommand", L_noop_ok },
    { "FilterPixel", L_noop_echo },
    { "GetCustomColourBackground", L_noop_ok },
    { "GetCustomColourName", L_noop_string },
    { "GetNoteColour", L_noop_neg },
    { "MoveNotepadWindow", L_noop_false },
    { "Redraw", L_noop_void },
    { "Repaint", L_noop_void },
    { "ResetIP", L_noop_void },
    { "SetCustomColourBackground", L_noop_void },
    { "SetCustomColourName", L_noop_ok },
    { "SetCustomColourText", L_noop_void },
    { "SetFrameBackgroundColour", L_noop_void },
    { "SetNoteColour", L_noop_void },
    { "Transparency", L_noop_false },
    { "WindowBezier", L_noop_ok },

    { nullptr, nullptr } };

namespace {
int
L_world_tostring(lua_State* L)
{
  BENCHMARK
  lua_pushliteral(L, "world");
  return 1;
}
} // namespace

static const struct luaL_Reg worldlib_meta[] = { { "__tostring",
                                                   L_world_tostring },
                                                 { nullptr, nullptr } };

int
registerLuaWorld(lua_State* L)
{
  luaL_newlib(L, worldlib);

  luaL_newmetatable(L, worldRegKey);
  luaL_setfuncs(L, worldlib_meta, 0);
  lua_setmetatable(L, -2);

  lua_setglobal(L, worldLibKey);

  lua_pushglobaltable(L);
  if (lua_getmetatable(L, -1) == LUA_TNIL) {
    lua_newtable(L);
  }

  lua_getglobal(L, worldLibKey);
  lua_setfield(L, -2, "__index");
  lua_setmetatable(L, -2);

  return 1;
}
