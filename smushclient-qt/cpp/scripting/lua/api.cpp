#include "api.h"
#include "../miniwindow/imagefilters.h"
#include "../qlua.h"
#include "../scriptapi.h"
#include "errors.h"
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

namespace {
const char* const indexRegKey = "smushclient.plugin";
const char* const worldRegKey = "smushclient.world";
const char* const worldLibKey = "world";
} // namespace

#define expect_nonnull(opt, code)                                              \
  if (!(opt)) [[unlikely]] {                                                   \
    return returnCode(L, (code));                                              \
  }

// Private localization

namespace {
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
  lua_pushinteger(L, static_cast<lua_Integer>(code));
  return 1;
}

inline int
returnCode(lua_State* L, ApiCode code, const QString& reason)
{
  lua_pushinteger(L, static_cast<lua_Integer>(code));
  qlua::pushQString(L, reason);
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
convertVolume(lua_Number decibels)
{
  return static_cast<float>(1 / pow(2, decibels / -3));
}

inline ScriptApi**
apiSlot(lua_State* L) noexcept
{
  return static_cast<ScriptApi**>(lua_getextraspace(L)); // NOLINT
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
  lua_pushinteger(L, static_cast<lua_Integer>(index));
  lua_rawsetp(L, LUA_REGISTRYINDEX, indexRegKey);
  return 0;
}

// benchmarking

#ifdef BENCHMARK_LUA
class Benchmarker
{
public:
  explicit Benchmarker(const char* name)
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

// database

int
L_DatabaseClose(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  lua_pushinteger(L, getApi(L).DatabaseClose(qlua::getString(L, 1)));
  return 1;
}

int
L_DatabaseOpen(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 3);
  lua_pushinteger(L,
                  getApi(L).DatabaseOpen(qlua::getString(L, 1),
                                         qlua::getString(L, 2),
                                         qlua::getInt(L, 3, 6)));
  return 1;
}

// info

int
L_GetInfo(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  qlua::pushQVariant(L, getApi(L).GetInfo(qlua::getInteger(L, 1)));
  return 1;
}

int
L_GetLineInfo(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  qlua::pushQVariant(
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
    qlua::pushQVariant(
      L, getApi(L).GetPluginInfo(pluginID, static_cast<uint8_t>(infoType)));
  }
  return 1;
}

int
L_GetStyleInfo(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 3);
  qlua::pushQVariant(L,
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
  qlua::pushQVariant(L,
                     getApi(L).GetTimerInfo(getPluginIndex(L),
                                            qlua::getString(L, 1),
                                            qlua::getInteger(L, 2)));
  return 1;
}

int
L_GetUniqueNumber(lua_State* L)
{
  lua_pushinteger(L, ScriptApi::GetUniqueNumber());
  return 1;
}

int
L_Version(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  qlua::pushString(L, SCRIPTING_VERSION);
  return 1;
}

int
L_WindowInfo(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  qlua::pushQVariant(
    L, getApi(L).WindowInfo(qlua::getString(L, 1), qlua::getInteger(L, 2)));
  return 1;
}

// input

int
L_Send(lua_State* L)
{
  BENCHMARK
  QByteArray bytes = qlua::concatBytes(L);
  return returnCode(L, getApi(L).Send(bytes));
}

int
L_SendNoEcho(lua_State* L)
{
  BENCHMARK
  QByteArray bytes = qlua::concatBytes(L);
  return returnCode(L, getApi(L).SendNoEcho(bytes));
}

int
L_SendPkt(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  return returnCode(L, getApi(L).SendPacket(qlua::getBytes(L, 1)));
}

// options

int
L_GetAlphaOption(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  const string_view option =
    getApi(L).GetAlphaOption(getPluginIndex(L), qlua::getString(L, 1));
  qlua::pushString(L, option);
  return 1;
}

int
L_GetAlphaOptionList(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  qlua::pushQStrings(L, ScriptApi::GetAlphaOptionList());
  return 1;
}

int
L_GetCurrentValue(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  qlua::pushQVariant(
    L, getApi(L).GetCurrentValue(getPluginIndex(L), qlua::getString(L, 1)));
  return 1;
}

int
L_GetOption(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  lua_pushinteger(
    L, getApi(L).GetOption(getPluginIndex(L), qlua::getString(L, 1)));
  return 1;
}

int
L_GetOptionList(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  qlua::pushQStrings(L, ScriptApi::GetOptionList());
  return 1;
}

int
L_SetAlphaOption(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  getApi(L).SetAlphaOption(
    getPluginIndex(L), qlua::getString(L, 1), qlua::getString(L, 2));
  return 1;
}

int
L_SetOption(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  getApi(L).SetOption(
    getPluginIndex(L), qlua::getString(L, 1), qlua::getIntegerOrBool(L, 2, 0));
  return 1;
}

// output

int
L_ColourNameToRGB(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  qlua::pushQColor(L, qlua::getQColor(L, 1));
  return 1;
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
  lua_pushinteger(L, getApi(L).GetLinesInBufferCount());
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
  api.Tell(qlua::concatStrings(L));
  api.finishNote();
  return 0;
}

int
L_PickColour(lua_State* L)
{
  BENCHMARK
  qlua::pushQColor(L, getApi(L).PickColour(qlua::getQColor(L, 1, QColor())));
  return 1;
}

int
L_RGBColourToName(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  qlua::pushQString(L, qlua::rgbCodeToColor(qlua::getInteger(L, 1)).name());
  return 1;
}

int
L_SetClipboard(lua_State* L)
{
  BENCHMARK
  ScriptApi::SetClipboard(qlua::concatStrings(L));
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
L_SetStatus(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  getApi(L).SetStatus(qlua::getQString(L, 1));
  return 0;
}

int
L_Simulate(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 1);
  getApi(L).Simulate(qlua::getString(L, 1));
  return 0;
}

int
L_Tell(lua_State* L)
{
  BENCHMARK
  getApi(L).Tell(qlua::concatStrings(L));
  return 0;
}

// plugins

int
L_BroadcastPlugin(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  lua_pushinteger(L,
                  getApi(L).BroadcastPlugin(getPluginIndex(L),
                                            qlua::getInteger(L, 1),
                                            qlua::getString(L, 2)));
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
    lua_pushinteger(
      L, static_cast<lua_Integer>(ApiCode::ErrorCallingPluginRoutine));
    qlua::pushQString(L, fmtCallError(plugin, routine));
    size_t size = 0;
    lua_pushlstring(L, lua_tolstring(L2, -1, &size), size);
    return 3;
  }

  const int topAfter = lua_gettop(L2);
  const int nresults = topAfter - topBefore;
  lua_settop(L, 0);
  luaL_checkstack(L, nresults + 1, nullptr);
  lua_pushinteger(L, static_cast<lua_Integer>(ApiCode::OK));
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
  qlua::pushString(L, getApi(L).GetPluginID(getPluginIndex(L)));
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

// senders

int
L_AddAlias(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 5);
  const string_view name = qlua::getString(L, 1);
  const string_view pattern = qlua::getString(L, 2);
  const string_view text = qlua::getString(L, 3);
  const QFlags<AliasFlag> flags = qlua::getFlags<AliasFlag>(L, 4);
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
  const QFlags<TimerFlag> flags = qlua::getFlags<TimerFlag>(L, 6);
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
  const QFlags<TriggerFlag> flags = qlua::getFlags<TriggerFlag>(L, 4);
  const QColor color = qlua::getCustomColor(L, 5);
  // const lua_Integer wildcardIndex = qlua::getInt(L, 6);
  const string_view soundFile = qlua::getString(L, 7);
  const optional<string_view> script = qlua::getScriptName(L, 8);
  const optional<SendTarget> target =
    qlua::getSendTarget(L, 9, SendTarget::World);
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
  lua_pushinteger(L,
                  static_cast<lua_Integer>(getApi(L).DeleteAliasGroup(
                    getPluginIndex(L), qlua::getString(L, 1))));
  return 1;
}

int
L_DeleteTemporaryAliases(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  lua_pushinteger(L,
                  static_cast<lua_Integer>(getApi(L).DeleteTemporaryAliases()));
  return 1;
}

int
L_DeleteTemporaryTimers(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  lua_pushinteger(L,
                  static_cast<lua_Integer>(getApi(L).DeleteTemporaryTimers()));
  return 1;
}

int
L_DeleteTemporaryTriggers(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 0);
  lua_pushinteger(
    L, static_cast<lua_Integer>(getApi(L).DeleteTemporaryTriggers()));
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
  lua_pushinteger(L,
                  static_cast<lua_Integer>(getApi(L).DeleteTimerGroup(
                    getPluginIndex(L), qlua::getString(L, 1))));
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
  lua_pushinteger(L,
                  static_cast<lua_Integer>(getApi(L).DeleteTriggerGroup(
                    getPluginIndex(L), qlua::getString(L, 1))));
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
  const optional<SendTarget> target = qlua::getSendTarget(L, 3);
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
  return returnCode(L,
                    getApi(L).EnableAliasGroup(getPluginIndex(L),
                                               qlua::getString(L, 1),
                                               qlua::getBool(L, 2, true)));
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
  return returnCode(L,
                    getApi(L).EnableTimerGroup(getPluginIndex(L),
                                               qlua::getString(L, 1),
                                               qlua::getBool(L, 2, true)));
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
  return returnCode(L,
                    getApi(L).EnableTrigger(getPluginIndex(L),
                                            qlua::getString(L, 1),
                                            qlua::getBool(L, 2, true)));
}

int
L_GetAliasOption(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  const size_t plugin = getPluginIndex(L);
  const string_view label = qlua::getString(L, 1);
  const string_view option = qlua::getString(L, 2);
  qlua::pushQVariant(L, getApi(L).GetAliasOption(plugin, label, option));
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
  qlua::pushQVariant(L, getApi(L).GetTimerOption(plugin, label, option));
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
  qlua::pushQVariant(L, getApi(L).GetTriggerOption(plugin, label, option));
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
  qlua::pushQString(L, ScriptApi::MakeRegularExpression(qlua::getString(L, 1)));
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

// variables

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
  qlua::pushVariable(
    L, getApi(L).GetVariable(getPluginIndex(L), qlua::getString(L, 1)));
  return 1;
}

int
L_GetPluginVariable(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 2);
  qlua::pushVariable(
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

// windows

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
L_WindowBlendImage(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 12);
  const string_view windowName = qlua::getString(L, 1);
  const string_view imageId = qlua::getString(L, 2);
  const QRectF rect = qlua::getQRectF(L, 3, 4, 5, 6);
  const optional<BlendMode> mode = qlua::getBlendMode(L, 7);
  const lua_Number opacity = qlua::getNumber(L, 8);
  const QRectF targetRect = qlua::getQRectF(L, 9, 10, 11, 12);
  expect_nonnull(mode, ApiCode::UnknownOption);
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
  const optional<CircleOp> action = qlua::getCircleOp(L, 2);
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
  const optional<MiniWindow::Position> position = qlua::getWindowPosition(L, 6);
  const MiniWindow::Flags flags = qlua::getFlags<MiniWindow::Flag>(L, 7);
  const QColor bg = qlua::getQColor(L, 8);
  expect_nonnull(position, ApiCode::BadParameter);
  return returnCode(
    L,
    getApi(L).WindowCreate(
      getPluginIndex(L), windowName, location, size, *position, flags, bg));
}

int
L_WindowDrawImage(lua_State* L)
{
  BENCHMARK
  const int n = expectMaxArgs(L, 11);
  const string_view windowName = qlua::getString(L, 1);
  const string_view imageID = qlua::getString(L, 2);
  const QRectF rect = qlua::getQRectF(L, 3, 4, 5, 6);
  const optional<MiniWindow::DrawImageMode> mode =
    qlua::getDrawImageMode(L, 7, MiniWindow::DrawImageMode::Copy);
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
    const optional<ffi::filter::Directions> directions =
      qlua::getDirections(L, 7, ImageFilter::Directions::Both);
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
  const optional<FilterOp> filterOp = qlua::getFilterOp(L, 6);
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
  const optional<QFont::StyleHint> hint =
    qlua::getFontHint(L, 10, QFont::StyleHint::AnyStyle);
  expect_nonnull(hint, ApiCode::BadParameter);
  return returnCode(L,
                    getApi(L).WindowFont(windowName,
                                         fontID,
                                         fontName,
                                         pointSize,
                                         bold,
                                         italic,
                                         underline,
                                         strikeout,
                                         *hint));
}

int
L_WindowFontInfo(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 3);
  qlua::pushQVariant(L,
                     getApi(L).WindowFontInfo(qlua::getString(L, 1),
                                              qlua::getString(L, 2),
                                              qlua::getInteger(L, 3)));
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
  const optional<Qt::Orientation> mode = qlua::getOrientation(L, 8);
  expect_nonnull(mode, ApiCode::UnknownOption);
  return returnCode(
    L, getApi(L).WindowGradient(windowName, rect, color1, color2, *mode));
}

int
L_WindowImageFromWindow(lua_State* L)
{
  BENCHMARK
  expectMaxArgs(L, 3);
  const string_view windowName = qlua::getString(L, 1);
  const string_view imageID = qlua::getString(L, 2);
  const string_view sourceWindow = qlua::getString(L, 3);
  return returnCode(
    L, getApi(L).WindowImageFromWindow(windowName, imageID, sourceWindow));
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
  const QVariant result = getApi(L).WindowMenu(
    qlua::getString(L, 1), qlua::getQPoint(L, 2, 3), qlua::getString(L, 4));
  qlua::pushQVariant(L, result);
  return 1;
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
  const optional<MiniWindow::Position> position = qlua::getWindowPosition(L, 4);
  const MiniWindow::Flags flags = qlua::getFlags<MiniWindow::Flag>(L, 5);
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
  const optional<RectOp> action = qlua::getRectOp(L, 2);
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
      if (optional<MiniWindow::ButtonFrame> frame = qlua::getButtonFrame(L, 7);
          frame) {
        return returnCode(
          L,
          getApi(L).WindowButton(windowName,
                                 rect.toRect(),
                                 *frame,
                                 qlua::getFlags<MiniWindow::ButtonFlag>(L, 8)));
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
  lua_pushinteger(L, static_cast<lua_Integer>(width));
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
  lua_pushinteger(L, width);
  return 1;
}

// window hotspots

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
  const Hotspot::Flags flags = qlua::getFlags(L, 14, Hotspot::Flags());
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

// userdata

int
L_noop(lua_State* L)
{
  return returnCode(L, ApiCode::OK);
}
} // namespace

static const struct luaL_Reg worldlib[] =
  // database
  { { "DatabaseClose", L_DatabaseClose },
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
    { "WindowInfo", L_WindowInfo },
    // input
    { "Send", L_Send },
    { "SendNoEcho", L_SendNoEcho },
    { "SendPkt", L_SendPkt },
    // options
    { "GetAlphaOption", L_GetAlphaOption },
    { "GetAlphaOptionList", L_GetAlphaOptionList },
    { "GetCurrentValue", L_GetCurrentValue },
    { "GetOption", L_GetOption },
    { "GetOptionList", L_GetOptionList },
    { "SetAlphaOption", L_SetAlphaOption },
    { "SetOption", L_SetOption },
    // output
    { "ColourNameToRGB", L_ColourNameToRGB },
    { "ColourNote", L_ColourNote },
    { "ColourTell", L_ColourTell },
    { "GetLinesInBufferCount", L_GetLinesInBufferCount },
    { "Hyperlink", L_Hyperlink },
    { "Note", L_Note },
    { "PickColour", L_PickColour },
    { "RGBColourToName", L_RGBColourToName },
    { "SetClipboard", L_SetClipboard },
    { "SetCursor", L_SetCursor },
    { "SetStatus", L_SetStatus },
    { "Simulate", L_Simulate },
    { "Tell", L_Tell },
    // plugins
    { "BroadcastPlugin", L_BroadcastPlugin },
    { "CallPlugin", L_CallPlugin },
    { "EnablePlugin", L_EnablePlugin },
    { "GetPluginID", L_GetPluginID },
    { "PluginSupports", L_PluginSupports },
    // senders
    { "AddAlias", L_AddAlias },
    { "AddTimer", L_AddTimer },
    { "AddTrigger", L_AddTrigger },
    { "AddTriggerEx", L_AddTrigger },
    { "DeleteAlias", L_DeleteAlias },
    { "DeleteAliasGroup", L_DeleteAliasGroup },
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
    { "EnableTimer", L_EnableTimer },
    { "EnableTimerGroup", L_EnableTimerGroup },
    { "EnableTrigger", L_EnableTrigger },
    { "EnableTriggerGroup", L_EnableTriggerGroup },
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
    // variables
    { "DeleteVariable", L_DeleteVariable },
    { "GetVariable", L_GetVariable },
    { "GetPluginVariable", L_GetPluginVariable },
    { "SetVariable", L_SetVariable },
    // windows
    { "TextRectangle", L_TextRectangle },
    { "WindowBlendImage", L_WindowBlendImage },
    { "WindowCircleOp", L_WindowCircleOp },
    { "WindowCreate", L_WindowCreate },
    { "WindowDrawImage", L_WindowDrawImage },
    { "WindowDrawImageAlpha", L_WindowDrawImageAlpha },
    { "WindowFilter", L_WindowFilter },
    { "WindowFont", L_WindowFont },
    { "WindowGradient", L_WindowGradient },
    { "WindowImageFromWindow", L_WindowImageFromWindow },
    { "WindowLine", L_WindowLine },
    { "WindowLoadImage", L_WindowLoadImage },
    { "WindowMenu", L_WindowMenu },
    { "WindowPolygon", L_WindowPolygon },
    { "WindowPosition", L_WindowPosition },
    { "WindowRectOp", L_WindowRectOp },
    { "WindowResize", L_WindowResize },
    { "WindowSetZOrder", L_WindowSetZOrder },
    { "WindowShow", L_WindowShow },
    { "WindowText", L_WindowText },
    { "WindowTextWidth", L_WindowTextWidth },
    // window hotspots
    { "WindowAddHotspot", L_WindowAddHotspot },
    { "WindowDeleteHotspot", L_WindowDeleteHotspot },
    { "WindowDragHandler", L_WindowDragHandler },
    { "WindowMoveHotspot", L_WindowMoveHotspot },
    { "WindowScrollwheelHandler", L_WindowScrollwheelHandler },
    // stubs
    { "SetBackgroundImage", L_noop },
    { "SetFrameBackgroundColour", L_noop },
    { "Redraw", L_noop },
    { "Repaint", L_noop },
    { "ResetIP", L_noop },

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
