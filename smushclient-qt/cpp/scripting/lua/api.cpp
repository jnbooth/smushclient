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

using qlua::colorToRgbCode;
using qlua::concatArgs;
using qlua::copyValue;
using qlua::expectMaxArgs;
using qlua::getBool;
using qlua::getBytes;
using qlua::getCursor;
using qlua::getCustomColor;
using qlua::getEnum;
using qlua::getInt;
using qlua::getInteger;
using qlua::getNumber;
using qlua::getOption;
using qlua::getQBrush;
using qlua::getQColor;
using qlua::getQFlags;
using qlua::getQFont;
using qlua::getQLineF;
using qlua::getQPen;
using qlua::getQPoint;
using qlua::getQPointF;
using qlua::getQPolygonF;
using qlua::getQRect;
using qlua::getQRectF;
using qlua::getQSize;
using qlua::getQString;
using qlua::getString;
using qlua::isScriptName;
using qlua::push;
using qlua::pushEntry;
using qlua::pushList;
using qlua::pushMap;
using qlua::rgbCodeToColor;

DECLARE_ENUM_BOUNDS(Qt::Orientation, Horizontal, Vertical)
DECLARE_ENUM_BOUNDS(SendTarget, World, ScriptAfterOmit)
DECLARE_ENUM_BOUNDS(ExportKind, Trigger, Keypad)

namespace {
const char* const callingRegKey = "smushclient.calling";
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

template<typename T>
QFlags<T>
combineFlags(std::initializer_list<std::pair<T, bool>> pairs)
{
  QFlags<T> flags;
  for (const std::pair<T, bool>& pair : pairs) {
    if (pair.second) {
      flags.setFlag(pair.first);
    }
  }
  return flags;
}

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
      return lua_tobool(L, idx) ? "1" : "0";
    case LUA_TNUMBER:
    case LUA_TSTRING:
      return lua_tostr(L, idx);
    default:
      return nullopt;
  }
}

inline void
insertTextTriples(lua_State* L, ScriptApi& api)
{
  int n = lua_gettop(L);
  for (int i = 1; i <= n; i += 3) {
    api.ColourTell(
      getQColor(L, i, {}), getQColor(L, i + 1, {}), getQString(L, i + 2));
  }
}

constexpr float
convertVolume(lua_Number decibels) noexcept
{
  return static_cast<float>(1.0 / pow(2, decibels / -3.0));
}

inline ScriptApi**
apiSlot(lua_State* L) noexcept
{
  return static_cast<ScriptApi**>(lua_getextraspace(L)); // NOLINT
}

void
pushVariable(lua_State* L, string_view variable)
{
  if (variable.data() != nullptr) {
    push(L, variable);
  }
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
  const QColor color = getQColor(L, 1);
  const optional<ColorAdjust> method = getEnum<ColorAdjust>(L, 2);
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
  push(L, getQColor(L, 1));
  return 1;
}

int
L_GetBoldColour(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  const lua_Integer i = getInteger(L, 1);
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
  push(L, getCustomColor(L, 1));
  return 1;
}

int
L_GetMapColour(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  const QColor color = getQColor(L, 1);
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
  const lua_Integer i = getInteger(L, 1);
  if (i >= 1 && i <= 8) {
    push(L, getApi(L).GetTermColour(static_cast<uint8_t>(i - 1)));
  } else {
    push(L, 0);
  }
  return 1;
}

int
L_GetNoteColourBack(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  getApi(L).GetOption(getPluginIndex(L), "note_background_colour");
  return 1;
}

int
L_GetNoteColourFore(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  getApi(L).GetOption(getPluginIndex(L), "note_text_colour");
  return 1;
}

int
L_GetSysColor(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  const optional<SysColor> sysColor = getEnum<SysColor>(L, 1);
  expect_nonnull(sysColor, ApiCode::BadParameter);
  push(L, ScriptApi::GetSysColor(*sysColor));
  return 1;
}

int
L_MapColour(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  getApi(L).MapColour(getQColor(L, 1), getQColor(L, 2));
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
    pushEntry(L, item.first, item.second);
  }
  return 1;
}

int
L_NoteColourRGB(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  ScriptApi& api = getApi(L);
  const size_t pluginIndex = getPluginIndex(L);
  if (QColor fore = getQColor(L, 1, {}); fore.isValid()) {
    api.SetOption(pluginIndex, "note_text_colour", colorToRgbCode(fore));
  }
  if (QColor back = getQColor(L, 2, {}); back.isValid()) {
    api.SetOption(pluginIndex, "note_background_colour", colorToRgbCode(back));
  }
  return 0;
}

int
L_PickColour(lua_State* L)
{
  BENCHMARK
  push(L, getApi(L).PickColour(getQColor(L, 1, {})));
  return 1;
}

int
L_RGBColourToName(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, rgbCodeToColor(getInteger(L, 1)).name());
  return 1;
}

int
L_SetBackgroundColour(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, getApi(L).SetBackgroundColour(getQColor(L, 1)));
  return 1;
}

int
L_SetBoldColour(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  const lua_Integer i = getInteger(L, 1);
  const QColor color = getQColor(L, 2);
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
  const lua_Integer i = getInteger(L, 1);
  const QColor color = getQColor(L, 2);
  if (i >= 1 && i <= 8) {
    getApi(L).SetTermColour(static_cast<uint8_t>(i - 1), color);
  }
  return 0;
}

int
L_SetNoteColourBack(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  getApi(L).SetOption(getPluginIndex(L),
                      "note_background_colour",
                      colorToRgbCode(getQColor(L, 1)));
  return 0;
}

int
L_SetNoteColourFore(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  getApi(L).SetOption(
    getPluginIndex(L), "note_text_colour", colorToRgbCode(getQColor(L, 1)));
  return 0;
}

// file

int
L_ChangeDir(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, ScriptApi::ChangeDir(getQString(L, 1)));
  return 1;
}

int
L_DatabaseClose(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, getApi(L).DatabaseClose(getString(L, 1)));
  return 1;
}

int
L_DatabaseOpen(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 3);
  push(
    L,
    getApi(L).DatabaseOpen(getString(L, 1), getString(L, 2), getInt(L, 3, 6)));
  return 1;
}

int
L_ExportXML(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  const optional<ExportKind> kind = getEnum<ExportKind>(L, 1);
  const string_view name = getString(L, 2);
  if (!kind) {
    lua_pushliteral(L, "");
    return 1;
  }
  push(L, getApi(L).ExportXML(getPluginIndex(L), *kind, name));
  return 1;
}

int
L_ImportXML(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, getApi(L).ImportXML(getString(L, 1)));
  return 1;
}

int
L_Save(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  push(L, getApi(L).Save(getQString(L, 1, {}), getBool(L, 2, false)));
  return 1;
}

int
L_SetChanged(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  getApi(L).SetChanged(getBool(L, 1, true));
  return 0;
}

// generate

int
L_CreateGUID(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  push(L, ScriptApi::CreateGUID());
  return 1;
}

int
L_GetUniqueID(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  push(L, ScriptApi::GetUniqueID());
  return 1;
}

int
L_GetUniqueNumber(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  push(L, ScriptApi::GetUniqueNumber());
  return 1;
}

int
L_MakeRegularExpression(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, ScriptApi::MakeRegularExpression(getString(L, 1)));
  return 1;
}

// info

int
L_GetAliasInfo(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  push(L,
       getApi(L).GetAliasInfo(
         getPluginIndex(L), getString(L, 1), getInteger(L, 2)));
  return 1;
}

int
L_GetInfo(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, getApi(L).GetInfo(getInteger(L, 1)));
  return 1;
}

int
L_GetLineInfo(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  push(L, getApi(L).GetLineInfo(getInt(L, 1) - 1, getInteger(L, 2)));
  return 1;
}

int
L_GetPluginAliasInfo(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 3);
  push(
    L,
    getApi(L).GetAliasInfo(getString(L, 1), getString(L, 2), getInteger(L, 3)));
  return 1;
}

int
L_GetPluginInfo(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  const string_view pluginID = getString(L, 1);
  const lua_Integer infoType = getInteger(L, 2);
  if (infoType > UINT8_MAX) [[unlikely]] {
    lua_pushnil(L);
    return 1;
  }
  if (infoType == 23) {
    lua_rawgetp(L, LUA_REGISTRYINDEX, callingRegKey);
    return 1;
  }
  push(L, getApi(L).GetPluginInfo(pluginID, static_cast<uint8_t>(infoType)));
  return 1;
}

int
L_GetPluginTimerInfo(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 3);
  push(
    L,
    getApi(L).GetTimerInfo(getString(L, 1), getString(L, 2), getInteger(L, 3)));
  return 1;
}

int
L_GetPluginTriggerInfo(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 3);
  push(L,
       getApi(L).GetTriggerInfo(
         getString(L, 1), getString(L, 2), getInteger(L, 3)));
  return 1;
}

int
L_GetStyleInfo(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 3);
  push(L,
       getApi(L).GetStyleInfo(
         getInt(L, 1) - 1, getInteger(L, 2) - 1, getInteger(L, 3)));
  return 1;
}

int
L_GetTimerInfo(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  push(L,
       getApi(L).GetTimerInfo(
         getPluginIndex(L), getString(L, 1), getInteger(L, 2)));
  return 1;
}

int
L_GetTriggerInfo(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  push(L,
       getApi(L).GetTriggerInfo(
         getPluginIndex(L), getString(L, 1), getInteger(L, 2)));
  return 1;
}

int
L_Hash(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, ScriptApi::Hash(getString(L, 1)));
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
  push(L,
       getApi(L).WindowFontInfo(
         getString(L, 1), getString(L, 2), getInteger(L, 3)));
  return 1;
}

int
L_WindowHotspotInfo(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 3);
  push(L,
       getApi(L).WindowHotspotInfo(
         getString(L, 1), getString(L, 2), getInteger(L, 3)));
  return 1;
}

int
L_WindowImageInfo(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 3);
  push(
    L,
    getApi(L).WindowImageInfo(getString(L, 1), getString(L, 2), getInt(L, 3)));
  return 1;
}

int
L_WindowInfo(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  push(L, getApi(L).WindowInfo(getString(L, 1), getInteger(L, 2)));
  return 1;
}

int
L_WorldName(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  push(L, getApi(L).GetOption(getPluginIndex(L), "name"));
  return 1;
}

// input

int
L_DeleteCommandHistory(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  getApi(L).DeleteCommandHistory();
  return 0;
}

int
L_DiscardQueue(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  push(L, getApi(L).DiscardQueue());
  return 1;
}

int
L_Execute(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  return returnCode(L, getApi(L).Execute(getQString(L, 1)));
}

int
L_EvaluateSpeedwalk(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, getApi(L).EvaluateSpeedwalk(getString(L, 1)));
  return 1;
}

int
L_GetCommand(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  push(L, getApi(L).GetCommand());
  return 1;
}

int
L_GetCommandList(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  const lua_Integer baseLimit = getInteger(L, 1, 0);
  const QStringList& log = getApi(L).GetCommandList();
  const lua_Integer size = static_cast<lua_Integer>(log.size());
  const lua_Integer limit = baseLimit == 0 ? size : std::min(baseLimit, size);
  lua_createtable(L, static_cast<int>(limit), 0);
  for (lua_Integer i = 1; i <= limit; ++i) {
    push(L, log.at(size - i));
    lua_rawseti(L, -2, i);
  }
  return 1;
}

int
L_GetQueue(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  const auto queue = getApi(L).GetQueue();
  lua_createtable(L, static_cast<int>(queue.size()), 0);
  lua_Integer i = 0;
  for (const auto& item : queue) {
    push(L, item.command);
    lua_rawseti(L, -2, ++i);
  }
  return 1;
}

int
L_GetSpeedWalkDelay(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  push(L, getApi(L).GetOption(getPluginIndex(L), "speed_walk_delay"));
  return 1;
}

int
L_LogSend(lua_State* L)
{
  BENCHMARK
  QByteArray bytes = concatArgs(L);
  return returnCode(L, getApi(L).LogSend(bytes));
}
int
L_PasteCommand(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, getApi(L).PasteCommand(getQString(L, 1)));
  return 1;
}

int
L_PushCommand(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  push(L, getApi(L).PushCommand());
  return 1;
}

int
L_Queue(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  return returnCode(L, getApi(L).Queue(getQString(L, 1), getBool(L, 2, true)));
}

int
L_ReverseSpeedwalk(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, ScriptApi::ReverseSpeedwalk(getString(L, 1)));
  return 1;
}

int
L_SelectCommand(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  getApi(L).PushCommand();
  return 0;
}

int
L_Send(lua_State* L)
{
  BENCHMARK
  QByteArray bytes = concatArgs(L);
  return returnCode(L, getApi(L).Send(bytes));
}

int
L_SendImmediate(lua_State* L)
{
  BENCHMARK
  QByteArray bytes = concatArgs(L);
  return returnCode(L, getApi(L).SendImmediate(bytes));
}

int
L_SendNoEcho(lua_State* L)
{
  BENCHMARK
  QByteArray bytes = concatArgs(L);
  return returnCode(L, getApi(L).SendNoEcho(bytes));
}

int
L_SendPkt(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  return returnCode(L, getApi(L).SendPacket(getBytes(L, 1)));
}

int
L_SendPush(lua_State* L)
{
  BENCHMARK
  QByteArray bytes = concatArgs(L);
  return returnCode(L, getApi(L).SendPush(bytes));
}

int
L_SetCommand(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  return returnCode(
    L, getApi(L).SetCommand(getQString(L, 1), getBool(L, 2, false)));
}

int
L_SetCommandSelection(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  return returnCode(
    L, getApi(L).SetCommandSelection(getInt(L, 1) - 1, getInt(L, 2) - 1));
}

int
L_SetCommandWindowHeight(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  return returnCode(L, getApi(L).SetCommandWindowHeight(getInt(L, 1)));
}

int
L_SetInputFont(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 4);
  QFont font = getQFont(L, 1);
  font.setPointSizeF(getNumber(L, 2));
  if (int weight = getInt(L, 3, 0)) {
    font.setWeight(QFont::Weight(weight));
  }
  font.setItalic(getBool(L, 4, false));
  getApi(L).SetInputFont(font);
  return 0;
}

int
L_SetSpeedWalkDelay(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  getApi(L).SetOption(getPluginIndex(L), "speed_walk_delay", getInteger(L, 1));
  return 0;
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
L_GetLogInput(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  push(L, getApi(L).GetOption(getPluginIndex(L), "log_input") != 0);
  return 1;
}

int
L_GetLogNotes(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  push(L, getApi(L).GetOption(getPluginIndex(L), "log_notes") != 0);
  return 1;
}

int
L_GetLogOutput(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  push(L, getApi(L).GetOption(getPluginIndex(L), "log_output") != 0);
  return 1;
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
    L, getApi(L).OpenLog(getString(L, 1, ""), getBool(L, 2, false)));
}

int
L_SetLogInput(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  getApi(L).SetOption(getPluginIndex(L),
                      "log_input",
                      static_cast<lua_Integer>(getBool(L, 1, true)));
  return 0;
}

int
L_SetLogNotes(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  getApi(L).SetOption(getPluginIndex(L),
                      "log_notes",
                      static_cast<lua_Integer>(getBool(L, 1, true)));
  return 0;
}

int
L_SetLogOutput(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  getApi(L).SetOption(getPluginIndex(L),
                      "log_output",
                      static_cast<lua_Integer>(getBool(L, 1, true)));
  return 0;
}

int
L_WriteLog(lua_State* L)
{
  BENCHMARK
  const QByteArray message = concatArgs(L);
  return returnCode(
    L, getApi(L).WriteLog(string_view(message.data(), message.size())));
}

// network

int
L_Connect(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  return returnCode(L, getApi(L).Connect());
}

int
L_Disconnect(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  return returnCode(L, getApi(L).Disconnect());
}

int
L_GetConnectDuration(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  push(L,
       std::chrono::duration_cast<std::chrono::seconds>(
         getApi(L).GetConnectDuration())
         .count());
  return 1;
}

int
L_GetHostAddress(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  pushList(L, ScriptApi::GetHostAddress(getQString(L, 1)));
  return 1;
}

int
L_GetHostName(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, ScriptApi::GetHostName(getQString(L, 1)));
  return 1;
}

int
L_GetReceivedBytes(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  push(L, getApi(L).GetReceivedBytes());
  return 1;
}

int
L_GetSentBytes(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  push(L, getApi(L).GetSentBytes());
  return 1;
}

int
L_IsConnected(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  push(L, getApi(L).IsConnected());
  return 1;
}

int
L_WorldAddress(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  push(L, getApi(L).WorldAddress());
  return 1;
}

int
L_WorldPort(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  push(L, getApi(L).WorldPort());
  return 1;
}

// note

int
L_AnsiNote(lua_State* L)
{
  BENCHMARK
  const QByteArray note = concatArgs(L);
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
L_GetNoteStyle(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  push(L, ScriptFont::styleFlags(getApi(L).GetNoteStyle()));
  return 1;
}

int
L_Hyperlink(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 7);
  getApi(L).Hyperlink(getQString(L, 1),
                      getQString(L, 2),
                      getQString(L, 3, {}),
                      getQColor(L, 4, {}),
                      getQColor(L, 5, {}),
                      getBool(L, 6, false),
                      getBool(L, 7, false));
  return 0;
}

int
L_Note(lua_State* L)
{
  BENCHMARK
  ScriptApi& api = getApi(L);
  api.Tell(QString::fromUtf8(concatArgs(L)));
  api.finishNote();
  return 0;
}

int
L_NoteHr(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  getApi(L).NoteHr();
  return 0;
}

int
L_NoteStyle(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  getApi(L).NoteStyle(
    ScriptFont::format(getQFlags<ScriptFont::StyleFlag>(L, 1)));
  return 0;
}

int
L_Tell(lua_State* L)
{
  BENCHMARK
  getApi(L).Tell(QString::fromUtf8(concatArgs(L)));
  return 0;
}

// notepad

int
L_ActivateNotepad(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, getApi(L).ActivateNotepad(getQString(L, 1)));
  return 1;
}

int
L_AppendToNotepad(lua_State* L)
{
  BENCHMARK
  push(L,
       getApi(L).AppendToNotepad(getQString(L, 1),
                                 QString::fromUtf8(concatArgs(L, 2))));
  return 1;
}

int
L_CloseNotepad(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  push(L, getApi(L).CloseNotepad(getQString(L, 1), getBool(L, 2, false)));
  return 1;
}

int
L_GetNotepadLength(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, getApi(L).GetNotepadLength(getQString(L, 1)));
  return 1;
}

int
L_GetNotepadList(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  pushList(L, getApi(L).GetNotepadList(getBool(L, 1, false)));
  return 1;
}

int
L_GetNotepadText(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, getApi(L).GetNotepadText(getQString(L, 1)));
  return 1;
}

int
L_GetNotepadWindowPosition(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, getApi(L).GetNotepadWindowPosition(getQString(L, 1)));
  return 1;
}

int
L_NotepadColour(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 3);
  push(L,
       getApi(L).NotepadColour(
         getQString(L, 1), getQColor(L, 2), getQColor(L, 3)));
  return 1;
}

int
L_NotepadFont(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 5);
  const QString title = getQString(L, 1);
  const QTextCharFormat format =
    Notepad::format(getQString(L, 2, {}),
                    getNumber(L, 3, 0.0),
                    getQFlags<Notepad::StyleFlag>(L, 4, {}));
  // const lua_Integer charset = getInteger(L, 5, 0);
  push(L, getApi(L).NotepadFont(title, format));
  return 1;
}

int
L_NotepadReadOnly(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  push(L, getApi(L).NotepadReadOnly(getQString(L, 1), getBool(L, 2, true)));
  return 1;
}

int
L_NotepadSaveMethod(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  const QString title = getQString(L, 1);
  const optional<Notepad::SaveMethod> method =
    getEnum<Notepad::SaveMethod>(L, 2);
  push(L, method && getApi(L).NotepadSaveMethod(title, *method));
  return 1;
}

int
L_ReplaceNotepad(lua_State* L)
{
  BENCHMARK
  push(L,
       getApi(L).ReplaceNotepad(getQString(L, 1),
                                QString::fromUtf8(concatArgs(L, 2))));
  return 1;
}

int
L_SendToNotepad(lua_State* L)
{
  BENCHMARK
  push(L,
       getApi(L).SendToNotepad(getQString(L, 1),
                               QString::fromUtf8(concatArgs(L, 2))));
  return 1;
}

// option

int
L_GetAliasOption(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  push(L,
       getApi(L).GetAliasOption(
         getPluginIndex(L), getString(L, 1), getString(L, 2)));
  return 1;
}

int
L_GetAlphaOption(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  pushVariable(L, getApi(L).GetAlphaOption(getPluginIndex(L), getString(L, 1)));
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
  push(L, getApi(L).GetCurrentValue(getPluginIndex(L), getString(L, 1)));
  return 1;
}

int
L_GetDefaultValue(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, ScriptApi::GetDefaultValue(getString(L, 1)));
  return 1;
}

int
L_GetOption(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, getApi(L).GetOption(getPluginIndex(L), getString(L, 1)));
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
L_GetTimerOption(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  push(L,
       getApi(L).GetTimerOption(
         getPluginIndex(L), getString(L, 1), getString(L, 2)));
  return 1;
}

int
L_GetTriggerOption(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  push(L,
       getApi(L).GetTriggerOption(
         getPluginIndex(L), getString(L, 1), getString(L, 2)));
  return 1;
}

int
L_SetAliasOption(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 3);
  const string_view label = getString(L, 1);
  const string_view option = getString(L, 2);
  const optional<string_view> value = getSenderOption(L, 3);
  expect_nonnull(value, ApiCode::OptionOutOfRange);
  return returnCode(
    L, getApi(L).SetAliasOption(getPluginIndex(L), label, option, *value));
}

int
L_SetAlphaOption(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  return returnCode(L,
                    getApi(L).SetAlphaOption(
                      getPluginIndex(L), getString(L, 1), getString(L, 2)));
}

int
L_SetOption(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  return returnCode(
    L,
    getApi(L).SetOption(getPluginIndex(L), getString(L, 1), getOption(L, 2)));
}

int
L_SetTimerOption(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 3);
  const string_view label = getString(L, 1);
  const string_view option = getString(L, 2);
  const optional<string_view> value = getSenderOption(L, 3);
  expect_nonnull(value, ApiCode::OptionOutOfRange);
  return returnCode(
    L, getApi(L).SetTimerOption(getPluginIndex(L), label, option, *value));
}

int
L_SetTriggerOption(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 3);
  const string_view label = getString(L, 1);
  const string_view option = getString(L, 2);
  const optional<string_view> value = getSenderOption(L, 3);
  expect_nonnull(value, ApiCode::OptionOutOfRange);
  return returnCode(
    L, getApi(L).SetTriggerOption(getPluginIndex(L), label, option, *value));
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
  return returnCode(L, ScriptApi::AddFont(getQString(L, 1)));
}

int
L_DeleteLines(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  getApi(L).DeleteLines(getInt(L, 1));
  return 0;
}

int
L_DeleteOutput(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  getApi(L).DeleteOutput();
  return 0;
}

int
L_FixupHTML(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  const rust::String fixed = ScriptApi::FixupHTML(getString(L, 1));
  if (fixed.empty()) {
    lua_pushvalue(L, 1);
  } else {
    push(L, fixed);
  }
  return 1;
}

int
L_GetEchoInput(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  push(L, getApi(L).GetEchoInput());
  return 1;
}

int
L_GetLineCount(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  push(L, getApi(L).GetInfo(201));
  return 1;
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
L_GetMainWindowPosition(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  // const bool differentCalc = getBool(L, 1, false);
  push(L, getApi(L).GetMainWindowPosition());
  return 1;
}

int
L_GetRecentLines(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, getApi(L).GetRecentLines(getInt(L, 1)));
  return 1;
}

int
L_GetWorldWindowPosition(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 3);
  // const int windowNum = getInt(L, 1, 1);
  // const bool useScreenCoords = getBool(L, 2, false);
  // const bool differentCalc = getBool(L, 3, false);
  push(L, getApi(L).GetWorldWindowPosition());
  return 1;
}

int
L_OpenBrowser(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  return returnCode(L, ScriptApi::OpenBrowser(getQString(L, 1)));
}

int
L_Pause(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  getApi(L).Pause(getBool(L, 1, true));
  return 0;
}

int
L_Reset(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  getApi(L).Reset();
  return 0;
}

int
L_ResetStatusTime(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  getApi(L).ResetStatusTime();
  return 0;
}

int
L_SetBackgroundImage(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  const QString path = getQString(L, 1, {});
  const optional<MiniWindow::Position> position =
    getEnum<MiniWindow::Position>(L, 2);
  expect_nonnull(position, ApiCode::BadParameter);
  return returnCode(L, getApi(L).SetBackgroundImage(path, *position));
}

int
L_SetCursor(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  const optional<Qt::CursorShape> cursor =
    getCursor(L, 1, Qt::CursorShape::ArrowCursor);
  expect_nonnull(cursor, ApiCode::BadParameter);
  return returnCode(L, getApi(L).SetCursor(*cursor));
}

int
L_SetEchoInput(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  getApi(L).SetEchoInput(getBool(L, 1, true));
  return 1;
}

int
L_SetForegroundImage(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  const QString path = getQString(L, 1);
  const optional<MiniWindow::Position> position =
    getEnum<MiniWindow::Position>(L, 2);
  expect_nonnull(position, ApiCode::BadParameter);
  return returnCode(L, getApi(L).SetForegroundImage(path, *position));
}

int
L_SetMainTitle(lua_State* L)
{
  BENCHMARK
  getApi(L).SetMainTitle(QString::fromUtf8(concatArgs(L)));
  return 0;
}

int
L_SetOutputFont(lua_State* L)
{
  BENCHMARK
  QFont font = getQFont(L, 1);
  font.setPointSizeF(getNumber(L, 2));
  getApi(L).SetOutputFont(font);
  return 0;
}

int
L_SetScroll(lua_State* L)
{
  BENCHMARK
  return returnCode(L, getApi(L).SetScroll(getInt(L, 1), getBool(L, 2, true)));
}

int
L_SetStatus(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  getApi(L).SetStatus(QString::fromUtf8(concatArgs(L)));
  return 0;
}

int
L_SetTitle(lua_State* L)
{
  BENCHMARK
  getApi(L).SetTitle(QString::fromUtf8(concatArgs(L)));
  return 0;
}

int
L_SetWorldWindowStatus(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  const optional<ScriptWindowStatus> status = getEnum<ScriptWindowStatus>(L, 1);
  if (!status) {
    return 0;
  }
  getApi(L).SetWorldWindowStatus(*status);
  return 0;
}

int
L_Simulate(lua_State* L)
{
  BENCHMARK
  const QByteArray message = concatArgs(L);
  getApi(L).Simulate(string_view(message.data(), message.size()));
  return 0;
}

int
L_TextRectangle(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 9);
  const QRect rect = getQRect(L, 1, 2, 3, 4);
  const int offset = getInt(L, 5);
  QColor borderColor = getQColor(L, 6);
  const int borderWidth = getInt(L, 7);
  optional<QBrush> outsideBrush =
    getQBrush(L, 8, 9, Qt::BrushStyle::SolidPattern);
  expect_nonnull(outsideBrush, ApiCode::BrushStyleNotValid);
  if (outsideBrush->color() == Qt::GlobalColor::black) {
    outsideBrush->setColor(Qt::GlobalColor::transparent);
  }
  if (borderColor == Qt::GlobalColor::black) {
    borderColor = Qt::GlobalColor::transparent;
  }
  return returnCode(L,
                    getApi(L).TextRectangle(
                      rect, offset, borderColor, borderWidth, *outsideBrush));
}

// plugin

int
L_BroadcastPlugin(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  push(L,
       getApi(L).BroadcastPlugin(
         getPluginIndex(L), getInteger(L, 1), getString(L, 2)));
  return 1;
}

int
L_CallPlugin(lua_State* L)
{
  BENCHMARK
  const Plugin* pluginRef = getApi(L).getPlugin(getString(L, 1));
  if (pluginRef == nullptr) [[unlikely]] {
    return returnCode(
      L, ApiCode::NoSuchPlugin, fmtNoSuchPlugin(getString(L, 1)));
  }

  const Plugin& plugin = *pluginRef;
  if (plugin.isDisabled()) [[unlikely]] {
    return returnCode(L, ApiCode::PluginDisabled, fmtPluginDisabled(plugin));
  }

  const string_view routine = getString(L, 2);

  const ScriptThread thread = plugin.spawnThread();
  lua_State* L2 = thread.state();
  push(L2, plugin.id());
  lua_rawsetp(L2, LUA_REGISTRYINDEX, callingRegKey);

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
    if (!copyValue(L, L2, i + 2)) [[unlikely]] {
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
    if (!copyValue(L2, L, i)) [[unlikely]] {
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
    L, getApi(L).EnablePlugin(getString(L, 1), getBool(L, 2, true)));
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
L_GetPluginList(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  pushList(L, getApi(L).GetPluginList());
  return 1;
}

int
L_GetPluginName(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  push(L, getApi(L).GetPluginName(getPluginIndex(L)));
  return 1;
}

int
L_IsPluginInstalled(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, getApi(L).IsPluginInstalled(getString(L, 1)));
  return 1;
}

int
L_PluginSupports(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  return returnCode(L,
                    getApi(L).PluginSupports(getString(L, 1), getString(L, 2)));
}

// selection

int
L_GetClipboard(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  push(L, ScriptApi::GetClipboard());
  return 1;
}

int
L_GetSelection(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  push(L, getApi(L).GetSelection());
  return 1;
}

int
L_GetSelectionEndColumn(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  push(L, getApi(L).GetSelectionEndColumn() + 1);
  return 1;
}

int
L_GetSelectionEndLine(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  push(L, getApi(L).GetSelectionEndLine() + 1);
  return 1;
}

int
L_GetSelectionStartColumn(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  push(L, getApi(L).GetSelectionEndColumn() + 1);
  return 1;
}

int
L_GetSelectionStartLine(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  push(L, getApi(L).GetSelectionEndLine() + 1);
  return 1;
}

int
L_SetClipboard(lua_State* L)
{
  BENCHMARK
  ScriptApi::SetClipboard(QString::fromUtf8(concatArgs(L)));
  return 0;
}

int
L_SetSelection(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 4);
  getApi(L).SetSelection(
    getInt(L, 1) - 1, getInt(L, 2) - 1, getInt(L, 3) - 1, getInt(L, 4) - 1);
  return 0;
}

// sender

int
L_AddAlias(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 5);
  const string_view name = getString(L, 1);
  const string_view pattern = getString(L, 2);
  const string_view text = getString(L, 3);
  const AliasFlags flags = getQFlags<AliasFlag>(L, 4);
  const string_view script = getString(L, 5, "");
  if (!script.empty() && !isScriptName(L, script)) {
    return returnCode(L, ApiCode::ScriptNameNotLocated);
  }
  return returnCode(
    L,
    getApi(L).AddAlias(getPluginIndex(L), name, pattern, text, flags, script));
}

int
L_AddTimer(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 7);
  const string_view name = getString(L, 1);
  const int hour = getInt(L, 2);
  const int minute = getInt(L, 3);
  const lua_Number second = getNumber(L, 4);
  const string_view text = getString(L, 5);
  const TimerFlags flags = getQFlags<TimerFlag>(L, 6);
  const string_view script = getString(L, 7, "");
  if (!script.empty() && !isScriptName(L, script)) {
    return returnCode(L, ApiCode::ScriptNameNotLocated);
  }

  return returnCode(
    L,
    getApi(L).AddTimer(
      getPluginIndex(L), name, hour, minute, second, text, flags, script));
}

int
L_AddTrigger(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 10);
  const string_view name = getString(L, 1);
  const string_view pattern = getString(L, 2);
  const string_view text = getString(L, 3);
  const TriggerFlags flags = getQFlags<TriggerFlag>(L, 4);
  const QColor color = getCustomColor(L, 5, {});
  const int clipboardArg = getInt(L, 6, 0);
  const string_view soundFile = getString(L, 7, "");
  const string_view script = getString(L, 8, "");
  const optional<SendTarget> target = getEnum(L, 9, SendTarget::World);
  const int sequence = getInt(L, 10, 100);

  if (!script.empty() && !isScriptName(L, script)) {
    return returnCode(L, ApiCode::ScriptNameNotLocated);
  }
  expect_nonnull(target, ApiCode::TriggerSendToInvalid);

  return returnCode(L,
                    getApi(L).AddTrigger(getPluginIndex(L),
                                         name,
                                         pattern,
                                         text,
                                         flags,
                                         color,
                                         clipboardArg,
                                         soundFile,
                                         script,
                                         *target,
                                         sequence));
}

int
L_DeleteAlias(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  return returnCode(L,
                    getApi(L).DeleteAlias(getPluginIndex(L), getString(L, 1)));
}

int
L_DeleteAliasGroup(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, getApi(L).DeleteAliasGroup(getPluginIndex(L), getString(L, 1)));
  return 1;
}

int
L_DeleteGroup(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  const size_t pluginIndex = getPluginIndex(L);
  ScriptApi& api = getApi(L);
  const string_view group = getString(L, 1);
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
  return returnCode(L,
                    getApi(L).DeleteTimer(getPluginIndex(L), getString(L, 1)));
}

int
L_DeleteTimerGroup(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, getApi(L).DeleteTimerGroup(getPluginIndex(L), getString(L, 1)));
  return 1;
}

int
L_DeleteTrigger(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  return returnCode(
    L, getApi(L).DeleteTrigger(getPluginIndex(L), getString(L, 1)));
}

int
L_DeleteTriggerGroup(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, getApi(L).DeleteTriggerGroup(getPluginIndex(L), getString(L, 1)));
  return 1;
}

int
L_DoAfter(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  const lua_Number seconds = getNumber(L, 1);
  const QString text = getQString(L, 2);
  return returnCode(
    L,
    getApi(L).DoAfter(getPluginIndex(L), seconds, text, SendTarget::Command));
}

int
L_DoAfterNote(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  const lua_Number seconds = getNumber(L, 1);
  const QString text = getQString(L, 2);
  return returnCode(
    L, getApi(L).DoAfter(getPluginIndex(L), seconds, text, SendTarget::Output));
}

int
L_DoAfterSpecial(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 3);
  const lua_Number seconds = getNumber(L, 1);
  const QString text = getQString(L, 2);
  const optional<SendTarget> target = getEnum<SendTarget>(L, 3);
  expect_nonnull(target, ApiCode::OptionOutOfRange);
  return returnCode(
    L, getApi(L).DoAfter(getPluginIndex(L), seconds, text, *target));
}

int
L_DoAfterSpeedwalk(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  const lua_Number seconds = getNumber(L, 1);
  const QString text = getQString(L, 2);
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
                    getApi(L).EnableAlias(
                      getPluginIndex(L), getString(L, 1), getBool(L, 2, true)));
}

int
L_EnableAliasGroup(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  push(L,
       getApi(L).EnableAliasGroup(
         getPluginIndex(L), getString(L, 1), getBool(L, 2, true)));
  return 1;
}

int
L_EnableGroup(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  const size_t pluginIndex = getPluginIndex(L);
  ScriptApi& api = getApi(L);
  const string_view group = getString(L, 1);
  const bool enable = getBool(L, 2, true);
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
                    getApi(L).EnableTimer(
                      getPluginIndex(L), getString(L, 1), getBool(L, 2, true)));
}

int
L_EnableTimerGroup(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  push(L,
       getApi(L).EnableTimerGroup(
         getPluginIndex(L), getString(L, 1), getBool(L, 2, true)));
  return 1;
}

int
L_EnableTrigger(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  return returnCode(L,
                    getApi(L).EnableTrigger(
                      getPluginIndex(L), getString(L, 1), getBool(L, 2, true)));
}

int
L_EnableTriggerGroup(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  push(L,
       getApi(L).EnableTriggerGroup(
         getPluginIndex(L), getString(L, 1), getBool(L, 2, true)));
  return 1;
}

int
L_GetAlias(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  const ScriptApi& api = getApi(L);
  const size_t pluginIndex = getPluginIndex(L);
  const string_view label = getString(L, 1);
  const ApiCode code = api.IsAlias(pluginIndex, label);
  push(L, code);
  if (code != ApiCode::OK) {
    return 1;
  }
  const Alias alias = api.GetAlias(pluginIndex, label);
  const SendTarget sendto = alias.getSendTo();
  push(L, alias.getPattern());
  push(L, alias.getText());
  push(L,
       combineFlags<AliasFlag>(
         { { AliasFlag::Enabled, alias.getEnabled() },
           { AliasFlag::KeepEvaluating, alias.getKeepEvaluating() },
           { AliasFlag::OmitFromLogFile, alias.getOmitFromLog() },
           { AliasFlag::RegularExpression, alias.getIsRegex() },
           { AliasFlag::ExpandVariables, alias.getExpandVariables() },
           { AliasFlag::AliasSpeedWalk, sendto == SendTarget::Speedwalk },
           { AliasFlag::AliasQueue, sendto == SendTarget::WorldDelay },
           { AliasFlag::AliasMenu, alias.getMenu() },
           { AliasFlag::Temporary, alias.getTemporary() },
           { AliasFlag::OneShot, alias.getOneShot() } }));
  push(L, alias.getScript());
  return 5;
}

int
L_GetAliasList(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  pushList(L, getApi(L).GetAliasList(getPluginIndex(L)));
  return 1;
}

int
L_GetAliasWildcard(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  push(L,
       getApi(L).GetAliasWildcard(
         getPluginIndex(L), getString(L, 1), getString(L, 2)));
  return 1;
}

int
L_GetPluginAliasList(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  pushList(L, getApi(L).GetAliasList(getString(L, 1)));
  return 1;
}

int
L_GetPluginTimerList(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  pushList(L, getApi(L).GetTimerList(getString(L, 1)));
  return 1;
}

int
L_GetPluginTriggerList(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  pushList(L, getApi(L).GetTriggerList(getString(L, 1)));
  return 1;
}

int
L_GetTimer(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  const ScriptApi& api = getApi(L);
  const size_t pluginIndex = getPluginIndex(L);
  const string_view label = getString(L, 1);
  const ApiCode code = api.IsTimer(pluginIndex, label);
  push(L, code);
  if (code != ApiCode::OK) {
    return 1;
  }
  const Timer timer = api.GetTimer(pluginIndex, label);
  const SendTarget sendto = timer.getSendTo();
  const Occurrence occurrence = timer.getOccurrence();
  int hour, minute, second, ms;
  switch (occurrence) {
    case Occurrence::Interval:
      hour = timer.getEveryHour();
      minute = timer.getEveryMinute();
      second = timer.getEverySecond();
      ms = timer.getEveryMillisecond();
      break;
    case Occurrence::Time: {
      const QTime& time = timer.getAtTime();
      hour = time.hour();
      minute = time.minute();
      second = time.second();
      ms = time.msec();
      break;
    }
  }
  push(L, hour);
  push(L, minute);
  push(L, ms / 1000.0 + second);
  push(L, timer.getText());
  push(L,
       combineFlags<TimerFlag>(
         { { TimerFlag::Enabled, timer.getEnabled() },
           { TimerFlag::AtTime, occurrence == Occurrence::Time },
           { TimerFlag::OneShot, timer.getOneShot() },
           { TimerFlag::TimerSpeedWalk, sendto == SendTarget::Speedwalk },
           { TimerFlag::TimerNote, sendto == SendTarget::Output },
           { TimerFlag::ActiveWhenClosed, timer.getActiveClosed() },
           { TimerFlag::Temporary, timer.getTemporary() } }));
  push(L, timer.getScript());
  return 7;
}

int
L_GetTrigger(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  const ScriptApi& api = getApi(L);
  const size_t pluginIndex = getPluginIndex(L);
  const string_view label = getString(L, 1);
  const ApiCode code = api.IsTrigger(pluginIndex, label);
  push(L, code);
  if (code != ApiCode::OK) {
    return 1;
  }
  const Trigger trigger = api.GetTrigger(pluginIndex, label);
  push(L, trigger.getPattern());
  push(L, trigger.getText());
  push(L,
       combineFlags<TriggerFlag>(
         { { TriggerFlag::Enabled, trigger.getEnabled() },
           { TriggerFlag::OmitFromLog, trigger.getOmitFromLog() },
           { TriggerFlag::OmitFromOutput, trigger.getOmitFromOutput() },
           { TriggerFlag::KeepEvaluating, trigger.getKeepEvaluating() },
           { TriggerFlag::IgnoreCase, trigger.getIgnoreCase() },
           { TriggerFlag::RegularExpression, trigger.getIsRegex() },
           { TriggerFlag::ExpandVariables, trigger.getExpandVariables() },
           { TriggerFlag::LowercaseWildcard, trigger.getLowercaseWildcard() },
           { TriggerFlag::Temporary, trigger.getTemporary() },
           { TriggerFlag::OneShot, trigger.getOneShot() } }));
  push(L, -1); // custom colour
  push(L, trigger.getClipboardArg());
  push(L, trigger.getSound());
  push(L, trigger.getScript());
  return 8;
}

int
L_GetTimerList(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  pushList(L, getApi(L).GetTimerList(getPluginIndex(L)));
  return 1;
}

int
L_GetTriggerList(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  pushList(L, getApi(L).GetTriggerList(getPluginIndex(L)));
  return 1;
}

int
L_GetTriggerWildcard(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  push(L,
       getApi(L).GetTriggerWildcard(
         getPluginIndex(L), getString(L, 1), getString(L, 2)));
  return 1;
}

int
L_IsAlias(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  return returnCode(L, getApi(L).IsAlias(getPluginIndex(L), getString(L, 1)));
}

int
L_IsTimer(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  return returnCode(L, getApi(L).IsTimer(getPluginIndex(L), getString(L, 1)));
}

int
L_IsTrigger(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  return returnCode(L, getApi(L).IsTrigger(getPluginIndex(L), getString(L, 1)));
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
L_GetSoundStatus(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, getApi(L).GetSoundStatus(getInteger(L, 1)));
  return 1;
}

int
L_PlaySound(lua_State* L)
{
  BENCHMARK
  // lua_Integer PlaySound(short Buffer, BSTR FileName, BOOL Loop, double
  // Volume, double Pan);
  expectMaxArgs(L, 5);
  return returnCode(L,
                    getApi(L).PlaySound(getInteger(L, 1),
                                        getString(L, 2),
                                        getBool(L, 3, false),
                                        convertVolume(getNumber(L, 4, 0.0))));
  // getDouble(L, 5) pan
}

int
L_PlaySoundMemory(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 5);
  return returnCode(
    L,
    getApi(L).PlaySoundMemory(getInteger(L, 1),
                              getBytes(L, 2),
                              getBool(L, 3, false),
                              convertVolume(getNumber(L, 4, 0.0))));
  // getDouble(L, 5) pan
}

int
L_StopSound(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  return returnCode(L, getApi(L).StopSound(getInteger(L, 1, 0)));
}

// variable

int
L_DeleteVariable(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  return returnCode(
    L, getApi(L).DeleteVariable(getPluginIndex(L), getString(L, 1)));
}

int
L_GetEntity(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, getApi(L).GetEntity(getString(L, 1)));
  return 1;
}

int
L_GetPluginVariable(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  pushVariable(L, getApi(L).GetVariable(getString(L, 1), getString(L, 2)));
  return 1;
}

int
L_GetPluginVariableList(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  pushMap(L, getApi(L).GetVariableList(getString(L, 1)));
  return 1;
}

int
L_GetXMLEntity(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  push(L, ScriptApi::GetXMLEntity(getString(L, 1)));
  return 1;
}

int
L_GetVariable(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  pushVariable(L, getApi(L).GetVariable(getPluginIndex(L), getString(L, 1)));
  return 1;
}

int
L_GetVariableList(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  pushMap(L, getApi(L).GetVariableList(getPluginIndex(L)));
  return 1;
}

int
L_SaveState(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  return returnCode(L, getApi(L).SaveState(getPluginIndex(L)));
}

int
L_SetEntity(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  getApi(L).SetEntity(getString(L, 1), getString(L, 2));
  return 0;
}

int
L_SetVariable(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  getApi(L).SetVariable(getPluginIndex(L), getString(L, 1), getString(L, 2));
  return returnCode(L, ApiCode::OK);
}

// window

int
L_WindowArc(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 12);
  const string_view windowName = getString(L, 1);
  const QRectF rect = getQRectF(L, 2, 3, 4, 5);
  const QPointF start = getQPointF(L, 6, 7);
  const QPointF end = getQPointF(L, 8, 9);
  const optional<QPen> pen = getQPen(L, 10, 11, 12);
  expect_nonnull(pen, ApiCode::PenStyleNotValid);
  return returnCode(L, getApi(L).WindowArc(windowName, rect, start, end, *pen));
}

int
L_WindowBlendImage(lua_State* L)
{
  BENCHMARK
  const int n = expectMaxArgs(L, 12);
  const string_view windowName = getString(L, 1);
  const string_view imageId = getString(L, 2);
  const QRectF rect = getQRectF(L, 3, 4, 5, 6);
  const optional<BlendMode> mode = getEnum<BlendMode>(L, 7);
  const lua_Number opacity = getNumber(L, 8);
  const QRectF targetRect = n > 8 ? getQRectF(L, 9, 10, 11, 12) : QRectF();
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
  const string_view windowName = getString(L, 1);
  const optional<CircleOp> action = getEnum<CircleOp>(L, 2);
  const QRectF rect = getQRectF(L, 3, 4, 5, 6);
  const optional<QPen> pen = getQPen(L, 7, 8, 9);
  const optional<QBrush> brush =
    getQBrush(L, 10, 11, Qt::BrushStyle::SolidPattern);
  expect_nonnull(action, ApiCode::UnknownOption);
  expect_nonnull(pen, ApiCode::PenStyleNotValid);
  expect_nonnull(brush, ApiCode::BrushStyleNotValid);

  switch (*action) {
    case CircleOp::Ellipse:
      return returnCode(
        L, getApi(L).WindowEllipse(windowName, rect, *pen, *brush));
    case CircleOp::Rectangle:
      return returnCode(L,
                        getApi(L).WindowRect(windowName, rect, *pen, *brush));
    case CircleOp::RoundedRectangle:
      return returnCode(L,
                        getApi(L).WindowRoundedRect(windowName,
                                                    rect,
                                                    getNumber(L, 12, 0.0),
                                                    getNumber(L, 13, 0.0),
                                                    *pen,
                                                    *brush));
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
  const string_view windowName = getString(L, 1);
  const QPoint location = getQPoint(L, 2, 3);
  const QSize size = getQSize(L, 4, 5);
  const optional<MiniWindow::Position> position =
    getEnum<MiniWindow::Position>(L, 6);
  const MiniWindow::Flags flags = getQFlags<MiniWindow::Flag>(L, 7);
  const QColor bg = getQColor(L, 8);
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
                    getApi(L).WindowCreateImage(getString(L, 1),
                                                getString(L, 2),
                                                { getInteger(L, 10),
                                                  getInteger(L, 9),
                                                  getInteger(L, 8),
                                                  getInteger(L, 7),
                                                  getInteger(L, 6),
                                                  getInteger(L, 5),
                                                  getInteger(L, 4),
                                                  getInteger(L, 3) }));
}

int
L_WindowDelete(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  return returnCode(L, getApi(L).WindowDelete(getString(L, 1)));
}

int
L_WindowDrawImage(lua_State* L)
{
  BENCHMARK
  const int n = expectMaxArgs(L, 11);
  const string_view windowName = getString(L, 1);
  const string_view imageID = getString(L, 2);
  const QRectF rect = getQRectF(L, 3, 4, 5, 6);
  const optional<DrawImageMode> mode = getEnum(L, 7, DrawImageMode::Copy);
  const QRectF sourceRect = n > 7 ? getQRectF(L, 8, 9, 10, 11) : QRectF();
  expect_nonnull(mode, ApiCode::BadParameter);
  return returnCode(
    L, getApi(L).WindowDrawImage(windowName, imageID, rect, *mode, sourceRect));
}

int
L_WindowDrawImageAlpha(lua_State* L)
{
  BENCHMARK
  const int n = expectMaxArgs(L, 9);
  const string_view windowName = getString(L, 1);
  const string_view imageID = getString(L, 2);
  const QRectF rect = getQRectF(L, 3, 4, 5, 6);
  const lua_Number opacity = getNumber(L, 7);
  const QPointF origin = n > 7 ? getQPointF(L, 8, 9) : QPointF();
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
      getEnum(L, 7, ImageFilter::Directions::Both);
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
  const string_view windowName = getString(L, 1);
  const QRect rect = getQRect(L, 2, 3, 4, 5);
  const FilterParams params{ .L = L, .windowName = windowName, .rect = rect };
  const optional<FilterOp> filterOp = getEnum<FilterOp>(L, 6);
  expect_nonnull(filterOp, ApiCode::UnknownOption);
  switch (*filterOp) {
    case FilterOp::Noise:
      return params.filter(ImageFilter::Noise(getNumber(L, 7)));
    case FilterOp::MonoNoise:
      return params.filter(ImageFilter::MonoNoise(getNumber(L, 7)));
    case FilterOp::Blur:
      return params.convolve<ImageFilter::Blur>();
    case FilterOp::Sharpen:
      return params.convolve<ImageFilter::Sharpen>();
    case FilterOp::EdgeDetect:
      return params.convolve<ImageFilter::EdgeDetect>();
    case FilterOp::Emboss:
      return params.convolve<ImageFilter::Emboss>();
    case FilterOp::BrightnessAdd:
      return params.filter(ImageFilter::BrightnessAdd(getInt(L, 7)));
    case FilterOp::Contrast:
      return params.filter(ImageFilter::Contrast(getNumber(L, 7)));
    case FilterOp::Gamma:
      return params.filter(ImageFilter::Gamma(getNumber(L, 7)));
    case FilterOp::RedBrightnessAdd:
      return params.filter(ImageFilter::BrightnessAdd(getInt(L, 7), Red));
    case FilterOp::RedContrast:
      return params.filter(ImageFilter::Contrast(getNumber(L, 7), Red));
    case FilterOp::RedGamma:
      return params.filter(ImageFilter::Gamma(getNumber(L, 7), Red));
    case FilterOp::GreenBrightnessAdd:
      return params.filter(ImageFilter::BrightnessAdd(getInt(L, 7), Green));
    case FilterOp::GreenContrast:
      return params.filter(ImageFilter::Contrast(getNumber(L, 7), Green));
    case FilterOp::GreenGamma:
      return params.filter(ImageFilter::Gamma(getNumber(L, 7), Green));
    case FilterOp::BlueBrightnessAdd:
      return params.filter(ImageFilter::BrightnessAdd(getInt(L, 7), Blue));
    case FilterOp::BlueContrast:
      return params.filter(ImageFilter::Contrast(getNumber(L, 7), Blue));
    case FilterOp::BlueGamma:
      return params.filter(ImageFilter::Gamma(getNumber(L, 7), Blue));
    case FilterOp::GrayscaleLinear:
      return params.filter(ImageFilter::GrayscaleLinear());
    case FilterOp::GrayscalePerceptual:
      return params.filter(ImageFilter::GrayscalePerceptual());
    case FilterOp::BrightnessMult:
      return params.filter(ImageFilter::BrightnessMult(getNumber(L, 7)));
    case FilterOp::RedBrightnessMult:
      return params.filter(ImageFilter::BrightnessMult(getNumber(L, 7), Red));
    case FilterOp::GreenBrightnessMult:
      return params.filter(ImageFilter::BrightnessMult(getNumber(L, 7), Green));
    case FilterOp::BlueBrightnessMult:
      return params.filter(ImageFilter::BrightnessMult(getNumber(L, 7), Blue));
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
  const string_view windowName = getString(L, 1);
  const string_view fontID = getString(L, 2);
  QFont font = getQFont(L, 3);
  const qreal pointSize = getNumber(L, 4);
  if (pointSize == 0 && font.family().isEmpty()) [[unlikely]] {
    return returnCode(L, getApi(L).WindowUnloadFont(windowName, fontID));
  }
  font.setPointSizeF(pointSize);
  font.setBold(getBool(L, 5, false));
  font.setItalic(getBool(L, 6, false));
  font.setUnderline(getBool(L, 7, false));
  font.setStrikeOut(getBool(L, 8, false));
  // const short charset = getInt(L, 9);
  const optional<ScriptFont> fontFlag =
    ScriptFont::validate(getInteger(L, 10, 0));
  expect_nonnull(fontFlag, ApiCode::BadParameter);
  font.setStyleHint(fontFlag->hint());
  return returnCode(L, getApi(L).WindowFont(windowName, fontID, font));
}

int
L_WindowFontList(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  pushList(L, getApi(L).WindowFontList(getString(L, 1)));
  return 1;
}

int
L_WindowGetImageAlpha(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 8);
  return returnCode(L,
                    getApi(L).WindowGetImageAlpha(getString(L, 1),
                                                  getString(L, 2),
                                                  getQRectF(L, 3, 4, 5, 6),
                                                  getQPointF(L, 7, 8)));
}

int
L_WindowGetPixel(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 3);
  const optional<QColor> optColor =
    getApi(L).WindowGetPixel(getString(L, 1), getQPoint(L, 2, 3));
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
  const string_view windowName = getString(L, 1);
  const QRectF rect = getQRectF(L, 2, 3, 4, 5);
  const QColor color1 = getQColor(L, 6);
  const QColor color2 = getQColor(L, 7);
  const optional<Qt::Orientation> mode = getEnum<Qt::Orientation>(L, 8);
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
                    getApi(L).WindowImageFromWindow(
                      getString(L, 1), getString(L, 2), getString(L, 3)));
}

int
L_WindowImageList(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  pushList(L, getApi(L).WindowImageList(getString(L, 1)));
  return 1;
}

int
L_WindowImageOp(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 13);
  const string_view windowName = getString(L, 1);
  const optional<ImageOp> action = getEnum<ImageOp>(L, 2);
  const QRectF rect = getQRectF(L, 3, 4, 5, 6);
  const optional<QPen> pen = getQPen(L, 7, 8, 9);
  const QColor color = getQColor(L, 10);
  const string_view imageID = getString(L, 11);
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
                                                    getNumber(L, 12),
                                                    getNumber(L, 13),
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
  const string_view windowName = getString(L, 1);
  const QLineF line = getQLineF(L, 2, 3, 4, 5);
  const optional<QPen> pen = getQPen(L, 6, 7, 8);
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
  const string_view windowName = getString(L, 1);
  const string_view imageID = getString(L, 2);
  const QString filename = getQString(L, 3);
  if (filename.isEmpty()) [[unlikely]] {
    return returnCode(L, getApi(L).WindowUnloadImage(windowName, imageID));
  }
  return returnCode(L,
                    getApi(L).WindowLoadImage(windowName, imageID, filename));
}

int
L_WindowLoadImageMemory(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 4);
  return returnCode(
    L,
    getApi(L).WindowLoadImageMemory(
      getString(L, 1), getString(L, 2), getBytes(L, 3), getBool(L, 4, false)));
}

int
L_WindowMenu(lua_State* L)
{
  expectMaxArgs(L, 4);
  push(
    L,
    getApi(L).WindowMenu(getString(L, 1), getQPoint(L, 2, 3), getString(L, 4)));
  return 1;
}

int
L_WindowMergeImageAlpha(lua_State* L)
{
  const int n = expectMaxArgs(L, 13);
  const string_view windowName = getString(L, 1);
  const string_view imageID = getString(L, 2);
  const string_view maskID = getString(L, 3);
  const QRect targetRect = getQRect(L, 4, 5, 6, 7);
  const optional<MergeMode> mode = getEnum<MergeMode>(L, 8);
  const qreal opacity = getNumber(L, 9);
  const QRect sourceRect = n > 9 ? getQRect(L, 10, 11, 12, 13) : QRect();
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
  const string_view windowName = getString(L, 1);
  const optional<QPolygonF> polygon = getQPolygonF(L, 2);
  const optional<QPen> pen = getQPen(L, 3, 4, 5);
  const optional<QBrush> brush =
    getQBrush(L, 6, 7, Qt::BrushStyle::SolidPattern);
  const bool close = getBool(L, 8, false);
  const Qt::FillRule fill = getBool(L, 9, false) ? Qt::FillRule::WindingFill
                                                 : Qt::FillRule::OddEvenFill;
  if (!polygon || polygon->size() < 2) [[unlikely]] {
    return returnCode(L, (ApiCode::InvalidNumberOfPoints));
  }
  expect_nonnull(pen, ApiCode::PenStyleNotValid);
  expect_nonnull(brush, ApiCode::BrushStyleNotValid);
  return returnCode(
    L,
    getApi(L).WindowPolygon(windowName, *polygon, *pen, *brush, close, fill));
}

int
L_WindowPosition(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 5);
  const string_view windowName = getString(L, 1);
  const QPoint location = getQPoint(L, 2, 3);
  const optional<MiniWindow::Position> position =
    getEnum<MiniWindow::Position>(L, 4);
  const MiniWindow::Flags flags = getQFlags<MiniWindow::Flag>(L, 5);
  expect_nonnull(position, ApiCode::BadParameter);
  return returnCode(
    L, getApi(L).WindowPosition(windowName, location, *position, flags));
}

int
L_WindowRectOp(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 8);
  const string_view windowName = getString(L, 1);
  const optional<RectOp> action = getEnum<RectOp>(L, 2);
  const QRectF rect = getQRectF(L, 3, 4, 5, 6);
  expect_nonnull(action, ApiCode::UnknownOption);
  switch (*action) {
    case RectOp::Frame:
      return returnCode(
        L, getApi(L).WindowRect(windowName, rect, getQColor(L, 7), QBrush()));
    case RectOp::Fill:
      return returnCode(
        L, getApi(L).WindowRect(windowName, rect, QPen(), getQColor(L, 7)));
    case RectOp::Invert:
      return returnCode(L, getApi(L).WindowInvert(windowName, rect.toRect()));
    case RectOp::Frame3D:
      return returnCode(
        L,
        getApi(L).WindowFrame(windowName,
                              rect,
                              getQColor(L, 7),
                              getQColor(L, 8, Qt::GlobalColor::black)));
    case RectOp::Edge3D:
      if (optional<ButtonFrame> frame = getEnum<ButtonFrame>(L, 7)) {
        return returnCode(
          L,
          getApi(L).WindowButton(windowName,
                                 rect.toRect(),
                                 *frame,
                                 getQFlags<MiniWindow::ButtonFlag>(L, 8)));
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
                    getApi(L).WindowResize(
                      getString(L, 1), getQSize(L, 2, 3), getQColor(L, 4)));
}

int
L_WindowSetPixel(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 3);
  return returnCode(L,
                    getApi(L).WindowSetPixel(
                      getString(L, 1), getQPoint(L, 2, 3), getQColor(L, 4)));
}

int
L_WindowSetZOrder(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  return returnCode(
    L, getApi(L).WindowSetZOrder(getString(L, 1), getInteger(L, 2)));
}

int
L_WindowShow(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  return returnCode(L,
                    getApi(L).WindowShow(getString(L, 1), getBool(L, 2, true)));
}

int
L_WindowText(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 9);
  const qreal width = getApi(L).WindowText(getString(L, 1),
                                           getString(L, 2),
                                           getString(L, 3),
                                           getQRectF(L, 4, 5, 6, 7),
                                           getQColor(L, 8),
                                           getBool(L, 9, false));
  push(L, width);
  return 1;
}

int
L_WindowTextWidth(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 4);
  push(
    L,
    getApi(L).WindowTextWidth(
      getString(L, 1), getString(L, 2), getString(L, 3), getBool(L, 4, false)));
  return 1;
}

int
L_WindowTransformImage(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 9);
  const string_view windowName = getString(L, 1);
  const string_view imageID = getString(L, 2);
  const lua_Integer modeN = getInteger(L, 5);
  const QTransform transform(getNumber(L, 6),
                             getNumber(L, 7),
                             getNumber(L, 8),
                             getNumber(L, 9),
                             getNumber(L, 3),
                             getNumber(L, 4));
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
  return returnCode(L,
                    getApi(L).WindowWrite(getString(L, 1), getQString(L, 2)));
}

// window hotspot

int
L_WindowAddHotspot(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 14);
  const string_view windowName = getString(L, 1);
  const string_view hotspotID = getString(L, 2);
  const QRect geometry = getQRectF(L, 3, 4, 5, 6).toRect();
  Hotspot::Callbacks callbacks{
    .mouseOver = string(getString(L, 7, "")),
    .cancelMouseOver = string(getString(L, 8, "")),
    .mouseDown = string(getString(L, 9, "")),
    .cancelMouseDown = string(getString(L, 10, "")),
    .mouseUp = string(getString(L, 11, "")),
  };
  const QString tooltip = getQString(L, 12, {});
  const optional<Qt::CursorShape> cursor =
    getCursor(L, 13, Qt::CursorShape::ArrowCursor);
  const Hotspot::Flags flags = getQFlags<Hotspot::Flag>(L, 14, {});
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
  return returnCode(L, getApi(L).WindowDeleteAllHotspots(getString(L, 1)));
}

int
L_WindowDeleteHotspot(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  return returnCode(
    L, getApi(L).WindowDeleteHotspot(getString(L, 1), getString(L, 2)));
}

int
L_WindowDragHandler(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 5);
  return returnCode(
    L,
    getApi(L).WindowUpdateHotspot(
      getPluginIndex(L),
      getString(L, 1),
      getString(L, 2),
      Hotspot::CallbacksPartial{ .dragMove = string(getString(L, 3, "")),
                                 .dragRelease = string(getString(L, 4, "")) }));
}

int
L_WindowHotspotList(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  pushList(L, getApi(L).WindowHotspotList(getString(L, 1)));
  return 1;
}

int
L_WindowHotspotTooltip(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 3);
  return returnCode(L,
                    getApi(L).WindowHotspotTooltip(
                      getString(L, 1), getString(L, 2), getQString(L, 3)));
}

int
L_WindowMoveHotspot(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 6);
  return returnCode(
    L,
    getApi(L).WindowMoveHotspot(
      getString(L, 1), getString(L, 2), getQRectF(L, 3, 4, 5, 6).toRect()));
}

int
L_WindowScrollwheelHandler(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 3);
  return returnCode(
    L,
    getApi(L).WindowUpdateHotspot(
      getPluginIndex(L),
      getString(L, 1),
      getString(L, 2),
      Hotspot::CallbacksPartial{ .scroll = string(getString(L, 3, "")) }));
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
NOOP(L_noop_zero, push(L, 0));

int
L_noop_void(lua_State* /* L */)
{
  return 0;
}

} // namespace

static constexpr const struct luaL_Reg worldlib[] =
  // color
  { { "AdjustColour", L_AdjustColour },
    { "ColourNameToRGB", L_ColourNameToRGB },
    { "GetBoldColour", L_GetBoldColour },
    { "GetCustomColourText", L_GetCustomColourText },
    { "GetMapColour", L_GetMapColour },
    { "GetNormalColour", L_GetNormalColour },
    { "GetNoteColourFore", L_GetNoteColourFore },
    { "GetNoteColourBack", L_GetNoteColourBack },
    { "GetSysColor", L_GetSysColor },
    { "MapColour", L_MapColour },
    { "MapColourList", L_MapColourList },
    { "NoteColourName", L_NoteColourRGB },
    { "NoteColourRGB", L_NoteColourRGB },
    { "PickColour", L_PickColour },
    { "RGBColourToName", L_RGBColourToName },
    { "SetBackgroundColour", L_SetBackgroundColour },
    { "SetBoldColour", L_SetBoldColour },
    { "SetNormalColour", L_SetNormalColour },
    { "SetNoteColourBack", L_SetNoteColourBack },
    { "SetNoteColourFore", L_SetNoteColourFore },
    // file
    { "ChangeDir", L_ChangeDir },
    { "DatabaseClose", L_DatabaseClose },
    { "DatabaseOpen", L_DatabaseOpen },
    { "ExportXML", L_ExportXML },
    { "ImportXML", L_ImportXML },
    { "Save", L_Save },
    { "SetChanged", L_SetChanged },
    // generate
    { "CreateGUID", L_CreateGUID },
    { "GetUniqueID", L_GetUniqueID },
    { "GetUniqueNumber", L_GetUniqueNumber },
    { "Hash", L_Hash },
    { "MakeRegularExpression", L_MakeRegularExpression },
    // info
    { "GetAliasInfo", L_GetAliasInfo },
    { "GetInfo", L_GetInfo },
    { "GetLineInfo", L_GetLineInfo },
    { "GetPluginAliasInfo", L_GetPluginAliasInfo },
    { "GetPluginInfo", L_GetPluginInfo },
    { "GetPluginTimerInfo", L_GetPluginTimerInfo },
    { "GetPluginTriggerInfo", L_GetPluginTriggerInfo },
    { "GetStyleInfo", L_GetStyleInfo },
    { "GetTimerInfo", L_GetTimerInfo },
    { "GetTriggerInfo", L_GetTriggerInfo },
    { "Version", L_Version },
    { "WindowFontInfo", L_WindowFontInfo },
    { "WindowHotspotInfo", L_WindowHotspotInfo },
    { "WindowImageInfo", L_WindowImageInfo },
    { "WindowInfo", L_WindowInfo },
    { "WorldName", L_WorldName },
    // input
    { "DeleteCommandHistory", L_DeleteCommandHistory },
    { "DiscardQueue", L_DiscardQueue },
    { "EvaluateSpeedwalk", L_EvaluateSpeedwalk },
    { "Execute", L_Execute },
    { "GetCommand", L_GetCommand },
    { "GetCommandList", L_GetCommandList },
    { "GetQueue", L_GetQueue },
    { "GetSpeedWalkDelay", L_GetSpeedWalkDelay },
    { "LogSend", L_LogSend },
    { "PasteCommand", L_PasteCommand },
    { "PushCommand", L_PushCommand },
    { "Queue", L_Queue },
    { "ReverseSpeedwalk", L_ReverseSpeedwalk },
    { "SelectCommand", L_SelectCommand },
    { "Send", L_Send },
    { "SendImmediate", L_SendImmediate },
    { "SendNoEcho", L_SendNoEcho },
    { "SendPkt", L_SendPkt },
    { "SendPush", L_SendPush },
    { "SetCommand", L_SetCommand },
    { "SetCommandSelection", L_SetCommandSelection },
    { "SetCommandWindowHeight", L_SetCommandWindowHeight },
    { "SetInputFont", L_SetInputFont },
    { "SetSpeedWalkDelay", L_SetSpeedWalkDelay },
    // log
    { "CloseLog", L_CloseLog },
    { "FlushLog", L_FlushLog },
    { "GetLogInput", L_GetLogInput },
    { "GetLogNotes", L_GetLogNotes },
    { "GetLogOutput", L_GetLogOutput },
    { "IsLogOpen", L_IsLogOpen },
    { "OpenLog", L_OpenLog },
    { "SetLogInput", L_SetLogInput },
    { "SetLogNotes", L_SetLogNotes },
    { "SetLogOutput", L_SetLogOutput },
    { "WriteLog", L_WriteLog },
    // network
    { "Connect", L_Connect },
    { "Disconnect", L_Disconnect },
    { "GetConnectDuration", L_GetConnectDuration },
    { "GetHostAddress", L_GetHostAddress },
    { "GetHostName", L_GetHostName },
    { "GetReceivedBytes", L_GetReceivedBytes },
    { "GetSentBytes", L_GetSentBytes },
    { "IsConnected", L_IsConnected },
    { "WorldAddress", L_WorldAddress },
    { "WorldPort", L_WorldPort },
    // note
    { "AnsiNote", L_AnsiNote },
    { "ColourNote", L_ColourNote },
    { "ColourTell", L_ColourTell },
    { "GetNoteStyle", L_GetNoteStyle },
    { "Hyperlink", L_Hyperlink },
    { "Note", L_Note },
    { "NoteHr", L_NoteHr },
    { "NoteStyle", L_NoteStyle },
    { "Tell", L_Tell },
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
    { "GetAliasOption", L_GetAliasOption },
    { "GetAlphaOption", L_GetAlphaOption },
    { "GetAlphaOptionList", L_GetAlphaOptionList },
    { "GetCurrentValue", L_GetCurrentValue },
    { "GetDefaultValue", L_GetDefaultValue },
    { "GetLoadedValue", L_GetCurrentValue },
    { "GetOption", L_GetOption },
    { "GetOptionList", L_GetOptionList },
    { "GetTimerOption", L_GetTimerOption },
    { "GetTriggerOption", L_GetTriggerOption },
    { "SetAliasOption", L_SetAliasOption },
    { "SetAlphaOption", L_SetAlphaOption },
    { "SetOption", L_SetOption },
    { "SetTimerOption", L_SetTimerOption },
    { "SetTriggerOption", L_SetTriggerOption },
    // output
    { "Activate", L_ActivateClient },
    { "ActivateClient", L_ActivateClient },
    { "AddFont", L_AddFont },
    { "DeleteLines", L_DeleteLines },
    { "DeleteOutput", L_DeleteOutput },
    { "FixupHTML", L_FixupHTML },
    { "GetEchoInput", L_GetEchoInput },
    { "GetLineCount", L_GetLineCount },
    { "GetLinesInBufferCount", L_GetLinesInBufferCount },
    { "GetMainWindowPosition", L_GetMainWindowPosition },
    { "GetRecentLines", L_GetRecentLines },
    { "GetWorldWindowPosition", L_GetWorldWindowPosition },
    { "OpenBrowser", L_OpenBrowser },
    { "Pause", L_Pause },
    { "Reset", L_Reset },
    { "ResetStatusTime", L_ResetStatusTime },
    { "SetBackgroundImage", L_SetBackgroundImage },
    { "SetCursor", L_SetCursor },
    { "SetEchoInput", L_SetEchoInput },
    { "SetForegroundImage", L_SetForegroundImage },
    { "SetMainTitle", L_SetMainTitle },
    { "SetOutputFont", L_SetOutputFont },
    { "SetScroll", L_SetScroll },
    { "SetStatus", L_SetStatus },
    { "SetTitle", L_SetTitle },
    { "SetWorldWindowStatus", L_SetWorldWindowStatus },
    { "Simulate", L_Simulate },
    { "TextRectangle", L_TextRectangle },
    // plugin
    { "BroadcastPlugin", L_BroadcastPlugin },
    { "CallPlugin", L_CallPlugin },
    { "EnablePlugin", L_EnablePlugin },
    { "GetPluginID", L_GetPluginID },
    { "GetPluginList", L_GetPluginList },
    { "GetPluginName", L_GetPluginName },
    { "IsPluginInstalled", L_IsPluginInstalled },
    { "PluginSupports", L_PluginSupports },
    // selection
    { "GetClipboard", L_GetClipboard },
    { "GetSelection", L_GetSelection },
    { "GetSelectionEndColumn", L_GetSelectionEndColumn },
    { "GetSelectionEndLine", L_GetSelectionEndLine },
    { "GetSelectionStartColumn", L_GetSelectionStartColumn },
    { "GetSelectionStartLine", L_GetSelectionStartLine },
    { "SetClipboard", L_SetClipboard },
    { "SetSelection", L_SetSelection },
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
    { "GetAlias", L_GetAlias },
    { "GetAliasList", L_GetAliasList },
    { "GetAliasWildcard", L_GetAliasWildcard },
    { "GetPluginAliasList", L_GetPluginAliasList },
    { "GetPluginTimerList", L_GetPluginTimerList },
    { "GetPluginTriggerList", L_GetPluginTriggerList },
    { "GetTimer", L_GetTimer },
    { "GetTimerList", L_GetTimerList },
    { "GetTrigger", L_GetTrigger },
    { "GetTriggerList", L_GetTriggerList },
    { "GetTriggerWildcard", L_GetTriggerWildcard },
    { "IsAlias", L_IsAlias },
    { "IsTimer", L_IsTimer },
    { "IsTrigger", L_IsTrigger },
    { "StopEvaluatingTriggers", L_StopEvaluatingTriggers },
    // sound
    { "GetSoundStatus", L_GetSoundStatus },
    { "PlaySound", L_PlaySound },
    { "PlaySoundMemory", L_PlaySoundMemory },
    { "StopSound", L_StopSound },
    // variable
    { "DeleteVariable", L_DeleteVariable },
    { "GetEntity", L_GetEntity },
    { "GetPluginVariable", L_GetPluginVariable },
    { "GetPluginVariableList", L_GetPluginVariableList },
    { "GetXMLEntity", L_GetXMLEntity },
    { "GetVariable", L_GetVariable },
    { "GetVariableList", L_GetVariableList },
    { "SaveState", L_SaveState },
    { "SetEntity", L_SetEntity },
    { "SetVariable", L_SetVariable },
    // window
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
    { "WindowLoadImageMemory", L_WindowLoadImageMemory },
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
    { "DoCommand", L_noop_ok },
    { "FilterPixel", L_noop_echo },
    { "FlashIcon", L_noop_void },
    { "GetCustomColourBackground", L_noop_zero },
    { "GetCustomColourName", L_noop_string },
    { "GetInternalCommandList", L_noop_empty },
    { "GetMapping", L_noop_false },
    { "GetMappingCount", L_noop_zero },
    { "GetMappingItem", L_noop_nil },
    { "GetMappingString", L_noop_string },
    { "GetNoteColour", L_noop_neg },
    { "GetNotes", L_noop_string },
    { "GetRemoveBacktracks", L_noop_false },
    { "GetRemoveMapReverses", L_noop_false },
    { "GetScriptTime", L_noop_zero },
    { "GetTrace", L_noop_false },
    { "GetUdpPort", L_noop_zero },
    { "Help", L_noop_void },
    { "Info", L_noop_void },
    { "InfoBackground", L_noop_void },
    { "InfoClear", L_noop_void },
    { "InfoColour", L_noop_void },
    { "InfoFont", L_noop_void },
    { "Mapping", L_noop_false },
    { "MoveMainWindow", L_noop_void },
    { "MoveNotepadWindow", L_noop_false },
    { "MoveWorldWindow", L_noop_void },
    { "MoveWorldWindowX", L_noop_void },
    { "ReadNamesFile", L_noop_ok },
    { "Redraw", L_noop_void },
    { "Repaint", L_noop_void },
    { "ResetIP", L_noop_void },
    { "SetCustomColourBackground", L_noop_void },
    { "SetCustomColourName", L_noop_ok },
    { "SetCustomColourText", L_noop_void },
    { "SetFrameBackgroundColour", L_noop_void },
    { "SetMapping", L_noop_void },
    { "SetNoteColour", L_noop_void },
    { "SetNotes", L_noop_void },
    { "SetRemoveBacktracks", L_noop_void },
    { "SetRemoveMapReverses", L_noop_void },
    { "SetToolBarPosition", L_noop_ok },
    { "SetTrace", L_noop_void },
    { "SetUnseenLines", L_noop_void },
    { "ShiftTabCompleteItem", L_noop_ok },
    { "ShowInfoBar", L_noop_void },
    { "SpellCheck", L_noop_nil },
    { "SpellCheckCommand", L_noop_spellcheck },
    { "SpellCheckDlg", L_noop_nil },
    { "TraceOut", L_noop_void },
    { "TranslateDebug", L_noop_ok },
    { "Transparency", L_noop_false },
    { "WindowBezier", L_noop_ok },

    { nullptr, nullptr } };

namespace {
// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
constexpr bool
charsEqual(const char* a, const char* b)
{
  for (;; ++a, ++b) {
    if (*a == 0 && *b == 0) {
      return true;
    }
    if (*a != *b) {
      return false;
    }
  }
}
constexpr const char*
findFirstDuplicate() noexcept
{
  constexpr const size_t size = sizeof(worldlib) / sizeof(luaL_Reg) - 1;
  constexpr const luaL_Reg* end = &worldlib[size];
  for (const luaL_Reg* it = worldlib; it < end; ++it) {
    const char* name = it->name;
    for (const luaL_Reg* compare = worldlib; compare < it; ++compare) {
      if (charsEqual(name, compare->name)) {
        return name;
      }
    }
  }
  return nullptr;
}
// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
constexpr const char* const firstDuplicate = findFirstDuplicate();
static_assert(firstDuplicate == nullptr);
} // namespace

namespace {
int
L_world_newindex(lua_State* L)
{
  lua_pushliteral(L, "attempt to update a read-only table");
  lua_error(L);
  return 0;
}

int
L_world_tostring(lua_State* L)
{
  lua_pushliteral(L, "world");
  return 1;
}
} // namespace

static const struct luaL_Reg worldlib_meta[] = {
  { "__newindex", L_world_newindex },
  { "__tostring", L_world_tostring },
  { nullptr, nullptr }
};

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
    lua_createtable(L, 0, 1);
  }

  lua_getglobal(L, worldLibKey);
  lua_setfield(L, -2, "__index");
  lua_setmetatable(L, -2);

  return 1;
}
