#include "luaapi.h"
#include <QtCore/QElapsedTimer>
#include <QtCore/QPointer>
#include "errors.h"
#include "imagefilters.h"
#include "qlua.h"
#include "scriptapi.h"
#include "scriptenums.h"
#include "scriptthread.h"
#include "worldproperties.h"
#include "smushclient_qt/src/bridge.cxxqt.h"
extern "C"
{
#include "lua.h"
#include "lauxlib.h"
}

static const char *apiRegKey = "smushclient.api";
static const char *indexRegKey = "smushclient.plugin";
static const char *worldRegKey = "smushclient.world";
static const char *worldLibKey = "world";

using std::nullopt;
using std::optional;
using std::string;
using std::string_view;
using std::unordered_map;
using stringmap = unordered_map<string, string>;

using qlua::expectMaxArgs;

static const unordered_map<string, TriggerBool> triggerBools =
    {
        {"enabled", TriggerBool::Enabled},
        {"expand_variables", TriggerBool::ExpandVariables},
        {"ignore_case", TriggerBool::IgnoreCase},
        {"keep_evaluating", TriggerBool::KeepEvaluating},
        {"lowercase_wildcard", TriggerBool::LowercaseWildcard},
        {"multi_line", TriggerBool::MultiLine},
        {"omit_from_log", TriggerBool::OmitFromLog},
        {"omit_from_output", TriggerBool::OmitFromOutput},
        {"one_shot", TriggerBool::OneShot},
        {"regexp", TriggerBool::IsRegex},
        {"repeat", TriggerBool::Repeats},
        {"sound_if_inactive", TriggerBool::SoundIfInactive},
};

// Private localization

QString convertStdString(const string &s)
{
  return QString::fromUtf8(s.data(), s.size());
}

QString fmtNoSuchPlugin(const QString &id)
{
  return ScriptApi::tr("Plugin ID (%1) is not installed")
      .arg(id);
}

QString fmtPluginDisabled(const Plugin &plugin)
{
  return ScriptApi::tr("Plugin '%1' (%2) is not enabled")
      .arg(convertStdString(plugin.name()))
      .arg(convertStdString(plugin.id()));
}

QString fmtNoSuchRoutine(const Plugin &plugin, string_view routine)
{
  return ScriptApi::tr("No function '%1' in plugin '%2' (%3)")
      .arg(QString::fromUtf8(routine.data(), routine.size()))
      .arg(convertStdString(plugin.name()))
      .arg(convertStdString(plugin.id()));
}

QString fmtBadParam(int idx, const char *type)
{
  return ScriptApi::tr("Cannot pass argument #%1 (%2 type) to CallPlugin")
      .arg(idx)
      .arg(QString::fromUtf8(type));
}

QString fmtCallError(const Plugin &plugin, string_view routine)
{
  return ScriptApi::tr("Runtime error in function '%1', plugin '%2' (%3)")
      .arg(QString::fromUtf8(routine.data(), routine.size()))
      .arg(convertStdString(plugin.name()))
      .arg(convertStdString(plugin.id()));
}

QString fmtBadReturn(const Plugin &plugin, string_view routine, int idx, const char *type)
{
  return ScriptApi::tr("Cannot handle return value #%1 (%2 type) from function '%3' in plugin '%4' (%5)")
      .arg(idx)
      .arg(QString::fromUtf8(type))
      .arg(QString::fromUtf8(routine.data(), routine.size()))
      .arg(convertStdString(plugin.name()))
      .arg(convertStdString(plugin.id()));
}

// Private utils

inline int returnCode(lua_State *L, ApiCode code)
{
  lua_pushinteger(L, (int)code);
  return 1;
}

inline int returnCode(lua_State *L, ApiCode code, const QString &reason)
{
  lua_pushinteger(L, (int)code);
  qlua::pushQString(L, reason);
  return 2;
}

int setLuaApi(lua_State *L, ScriptApi *api)
{
  lua_pushlightuserdata(L, api);
  lua_rawsetp(L, LUA_REGISTRYINDEX, apiRegKey);
  return 0;
}

inline ScriptApi &getApi(lua_State *L)
{
  lua_rawgetp(L, LUA_REGISTRYINDEX, apiRegKey);
  void *api = lua_touserdata(L, -1);
  lua_pop(L, 1);
  return *static_cast<ScriptApi *>(api);
}

int setPluginIndex(lua_State *L, size_t index)
{
  lua_pushinteger(L, index);
  lua_rawsetp(L, LUA_REGISTRYINDEX, indexRegKey);
  return 0;
}

inline size_t getPluginIndex(lua_State *L)
{
  lua_rawgetp(L, LUA_REGISTRYINDEX, indexRegKey);
  const size_t index = lua_tointeger(L, -1);
  lua_pop(L, 1);
  return index;
}

constexpr double convertVolume(double decibels) noexcept
{
  return 1 / pow(2, decibels / -3);
}

// benchmarking

#ifdef BENCHMARK_LUA
class Benchmarker
{
public:
  Benchmarker(const char *name) : name(name), timer()
  {
    timer.start();
  }
  ~Benchmarker()
  {
    int64_t elapsed = timer.elapsed();
    if (elapsed > 0)
      qDebug() << name << elapsed << "ms";
  }

private:
  const char *name;
  QElapsedTimer timer;
};

#define API(name) Benchmarker benchmarker(name);
#else
#define API(name)
#endif

// database

static int L_DatabaseClose(lua_State *L)
{
  API("DatabaseClose")
  expectMaxArgs(L, 1);
  lua_pushinteger(L, getApi(L).DatabaseClose(qlua::getString(L, 1)));
  return 1;
}

static int L_DatabaseOpen(lua_State *L)
{
  API("DatabaseOpen")
  expectMaxArgs(L, 3);
  lua_pushinteger(
      L,
      getApi(L).DatabaseOpen(qlua::getString(L, 1), qlua::getString(L, 2), qlua::getInt(L, 3, 6)));
  return 1;
}

// info

static int L_GetInfo(lua_State *L)
{
  API("GetInfo")
  expectMaxArgs(L, 1);
  qlua::pushQVariant(L, getApi(L).GetInfo(qlua::getInt(L, 1)));
  return 1;
}

static int L_GetLineInfo(lua_State *L)
{
  API("GetLineInfo")
  expectMaxArgs(L, 2);
  qlua::pushQVariant(L, getApi(L).GetLineInfo(qlua::getInt(L, 1) - 1, qlua::getInt(L, 2)));
  return 1;
}

static int L_GetStyleInfo(lua_State *L)
{
  API("GetStyleInfo")
  expectMaxArgs(L, 3);
  qlua::pushQVariant(
      L,
      getApi(L).GetStyleInfo(qlua::getInt(L, 1), qlua::getInt(L, 2), qlua::getInt(L, 3)));
  return 1;
}

static int L_Version(lua_State *L)
{
  API("Version")
  expectMaxArgs(L, 0);
  qlua::pushString(L, SCRIPTING_VERSION);
  return 1;
}

static int L_WindowInfo(lua_State *L)
{
  API("WindowInfo")
  expectMaxArgs(L, 2);
  qlua::pushQVariant(L, getApi(L).WindowInfo(qlua::getString(L, 1), qlua::getInt(L, 2)));
  return 1;
}

// input

static int L_Send(lua_State *L)
{
  API("Send")
  QByteArray bytes = qlua::concatBytes(L);
  return returnCode(L, getApi(L).Send(bytes));
}

static int L_SendNoEcho(lua_State *L)
{
  API("SendNoEcho")
  QByteArray bytes = qlua::concatBytes(L);
  return returnCode(L, getApi(L).SendNoEcho(bytes));
}

static int L_SendPkt(lua_State *L)
{
  API("SendPkt")
  expectMaxArgs(L, 1);
  return returnCode(L, getApi(L).SendPacket(qlua::getBytes(L, 1)));
}

// options

static int L_GetAlphaOption(lua_State *L)
{
  API("GetAlphaOption")
  expectMaxArgs(L, 1);
  const QVariant option = getApi(L).GetOption(qlua::getString(L, 1));
  QString value = option.toString();
  if (value.isEmpty() && option.canConvert<QColor>())
    value = option.value<QColor>().name();
  qlua::pushQString(L, value);
  return 1;
}

static int L_GetAlphaOptionList(lua_State *L)
{
  API("GetAlphaOptionList")
  expectMaxArgs(L, 0);
  qlua::pushStrings(L, WorldProperties::stringOptions());
  return 1;
}

static int L_GetCurrentValue(lua_State *L)
{
  API("GetCurrentValue")
  expectMaxArgs(L, 1);
  qlua::pushQVariant(L, getApi(L).GetOption(qlua::getString(L, 1)), true);
  return 1;
}

static int L_GetOption(lua_State *L)
{
  API("GetOption")
  expectMaxArgs(L, 1);
  const QVariant option = getApi(L).GetOption(qlua::getString(L, 1));
  const QMetaType type = option.metaType();
  switch (type.id())
  {
  case QMetaType::Double:
  case QMetaType::Float:
  case QMetaType::Float16:
    lua_pushnumber(L, option.toDouble());
    break;
  case QMetaType::QColor:
    qlua::pushQColor(L, option.value<QColor>());
    break;
  default:
    if (option.canConvert<int>())
      lua_pushinteger(L, option.value<int>());
    else
      lua_pushinteger(L, -1);
  }
  return 1;
}

static int L_GetOptionList(lua_State *L)
{
  API("GetOptionList")
  expectMaxArgs(L, 0);
  qlua::pushStrings(L, WorldProperties::numericOptions());
  return 1;
}

static int L_SetAlphaOption(lua_State *L)
{
  API("SetAlphaOption")
  expectMaxArgs(L, 2);
  getApi(L).SetOption(qlua::getString(L, 1), qlua::getQString(L, 2));
  return 1;
}

static int L_SetOption(lua_State *L)
{
  API("SetOption")
  expectMaxArgs(L, 2);
  getApi(L).SetOption(qlua::getString(L, 1), qlua::getNumberOrBool(L, 2, 0));
  return 1;
}

// output

inline void insertTextTriples(lua_State *L, ScriptApi &api)
{
  int n = lua_gettop(L);
  for (int i = 1; i <= n; i += 3)
    api.ColourTell(
        qlua::getQColor(L, i),
        qlua::getQColor(L, i + 1),
        qlua::getQString(L, i + 2));
}

static int L_ColourNameToRGB(lua_State *L)
{
  API("ColourNameToRGB")
  expectMaxArgs(L, 1);
  qlua::pushQColor(L, qlua::getQColor(L, 1));
  return 1;
}

static int L_ColourNote(lua_State *L)
{
  API("ColourNote")
  ScriptApi &api = getApi(L);
  insertTextTriples(L, api);
  api.finishNote();
  return 0;
}

static int L_ColourTell(lua_State *L)
{
  API("ColourTell")
  insertTextTriples(L, getApi(L));
  return 0;
}

static int L_GetLinesInBufferCount(lua_State *L)
{
  API("GetLinesInBufferCount")
  expectMaxArgs(L, 0);
  lua_pushinteger(L, getApi(L).GetLinesInBufferCount());
  return 1;
}

static int L_Hyperlink(lua_State *L)
{
  API("Hyperlink")
  expectMaxArgs(L, 7);
  getApi(L).Hyperlink(
      qlua::getQString(L, 1),
      qlua::getQString(L, 2),
      qlua::getQString(L, 3),
      qlua::getQColor(L, 4),
      qlua::getQColor(L, 5),
      qlua::getBool(L, 6, false),
      qlua::getBool(L, 7, false));
  return 0;
}

static int L_Note(lua_State *L)
{
  API("Note")
  ScriptApi &api = getApi(L);
  api.Tell(qlua::concatStrings(L));
  api.finishNote();
  return 0;
}

static int L_PickColour(lua_State *L)
{
  API("PickColour")
  qlua::pushQColor(L, getApi(L).PickColour(qlua::getQColor(L, 1, QColor())));
  return 1;
}

static int L_RGBColourToName(lua_State *L)
{
  API("RGBColourToName")
  expectMaxArgs(L, 1);
  qlua::pushQString(L, qlua::rgbCodeToColor(qlua::getInt(L, 1)).name());
  return 1;
}

static int L_SetClipboard(lua_State *L)
{
  API("SetClipboard")
  ScriptApi::SetClipboard(qlua::concatStrings(L));
  return 0;
}

static int L_SetCursor(lua_State *L)
{
  API("SetCursor")
  expectMaxArgs(L, 1);
  const optional<Qt::CursorShape> cursor = qlua::getCursor(L, 1, Qt::CursorShape::ArrowCursor);
  if (!cursor)
    return returnCode(L, ApiCode::BadParameter);
  return returnCode(L, getApi(L).SetCursor(*cursor));
}

static int L_SetStatus(lua_State *L)
{
  API("SetStatus")
  expectMaxArgs(L, 1);
  getApi(L).SetStatus(qlua::getQString(L, 1));
  return 0;
}

static int L_Tell(lua_State *L)
{
  API("Tell")
  getApi(L).Tell(qlua::concatStrings(L));
  return 0;
}

// plugins

static int L_BroadcastPlugin(lua_State *L)
{
  API("BroadcastPlugin")
  expectMaxArgs(L, 2);
  lua_pushinteger(
      L,
      getApi(L).BroadcastPlugin(getPluginIndex(L), qlua::getInt(L, 1), qlua::getString(L, 2)));
  return 1;
}

static int L_CallPlugin(lua_State *L)
{
  API("CallPlugin")
  const Plugin *pluginRef = getApi(L).getPlugin(qlua::getString(L, 1));
  if (!pluginRef) [[unlikely]]
    return returnCode(
        L,
        ApiCode::NoSuchPlugin,
        fmtNoSuchPlugin(qlua::getQString(L, 1)));

  const Plugin &plugin = *pluginRef;
  if (plugin.disabled()) [[unlikely]]
    return returnCode(L, ApiCode::PluginDisabled, fmtPluginDisabled(plugin));

  const string_view routine = qlua::getString(L, 2);

  const ScriptThread thread(plugin.state());
  lua_State *L2 = thread.state();

  const int nargs = lua_gettop(L) - 2;
  luaL_checkstack(L2, nargs + 1, nullptr);

  if (lua_getglobal(L2, routine.data()) != LUA_TFUNCTION)
  {
    lua_pop(L2, 1);
    return returnCode(L, ApiCode::NoSuchRoutine, fmtNoSuchRoutine(plugin, routine));
  }

  const int topBefore = lua_gettop(L2) - 1;

  for (int i = 1; i <= nargs; ++i)
    if (!qlua::copyValue(L, L2, i + 2))
    {
      lua_settop(L, 0);
      return returnCode(L, ApiCode::BadParameter, fmtBadParam(i - 2, luaL_typename(L, i)));
    }

  if (!api_pcall(L2, nargs, LUA_MULTRET))
  {
    lua_settop(L, 0);
    lua_pushinteger(L, (int)ApiCode::ErrorCallingPluginRoutine);
    qlua::pushQString(L, fmtCallError(plugin, routine));
    size_t size = 0;
    lua_pushlstring(L, lua_tolstring(L2, -1, &size), size);
    return 3;
  }

  const int topAfter = lua_gettop(L2);
  const int nresults = topAfter - topBefore;
  lua_settop(L, 0);
  luaL_checkstack(L, nresults + 1, nullptr);
  lua_pushinteger(L, (int)ApiCode::OK);
  for (int i = topBefore + 1; i <= topAfter; ++i)
    if (!qlua::copyValue(L2, L, i))
      return returnCode(L, ApiCode::ErrorCallingPluginRoutine,
                        fmtBadReturn(plugin, routine, i, luaL_typename(L2, i)));
  return nresults + 1;
}

static int L_EnablePlugin(lua_State *L)
{
  API("EnablePlugin")
  expectMaxArgs(L, 2);
  return returnCode(L, getApi(L).EnablePlugin(qlua::getString(L, 1), qlua::getBool(L, 2, true)));
}

static int L_GetPluginID(lua_State *L)
{
  API("GetPluginID")
  expectMaxArgs(L, 0);
  qlua::pushString(L, getApi(L).GetPluginID(getPluginIndex(L)));
  return 1;
}

static int L_GetPluginInfo(lua_State *L)
{
  API("GetPluginInfo")
  expectMaxArgs(L, 2);
  const string_view pluginID = qlua::getString(L, 1);
  const int infoType = qlua::getInt(L, 2);
  if (infoType > UINT8_MAX) [[unlikely]]
    lua_pushnil(L);
  else
    qlua::pushQVariant(L, getApi(L).GetPluginInfo(pluginID, (uint8_t)infoType));
  return 1;
}

static int L_PluginSupports(lua_State *L)
{
  API("PluginSupports")
  expectMaxArgs(L, 2);
  return returnCode(L, getApi(L).PluginSupports(qlua::getString(L, 1), qlua::getString(L, 2)));
}

// senders

static int L_AddAlias(lua_State *L)
{
  API("AddAlias")
  expectMaxArgs(L, 5);
  const QString name = qlua::getQString(L, 1);
  const QString pattern = qlua::getQString(L, 2);
  const QString text = qlua::getQString(L, 3);
  const QFlags<AliasFlag> flags = (QFlags<AliasFlag>)qlua::getInt(L, 4);
  const optional<QString> script = qlua::getScriptName(L, 5);

  if (!script)
    return returnCode(L, ApiCode::ScriptNameNotLocated);

  return returnCode(L, getApi(L).AddAlias(getPluginIndex(L), name, pattern, text, flags, *script));
}

static int L_AddTimer(lua_State *L)
{
  API("AddTimer")
  expectMaxArgs(L, 7);
  const QString name = qlua::getQString(L, 1);
  const int hour = qlua::getInt(L, 2);
  const int minute = qlua::getInt(L, 3);
  const double second = qlua::getNumber(L, 4);
  const QString text = qlua::getQString(L, 5);
  const QFlags<TimerFlag> flags = (QFlags<TimerFlag>)qlua::getInt(L, 6);
  const optional<QString> script = qlua::getScriptName(L, 7);

  if (!script)
    return returnCode(L, ApiCode::ScriptNameNotLocated);

  return returnCode(
      L,
      getApi(L).AddTimer(getPluginIndex(L), name, hour, minute, second, text, flags, *script));
}

static int L_AddTrigger(lua_State *L)
{
  API("AddTrigger")
  expectMaxArgs(L, 10);
  const QString name = qlua::getQString(L, 1);
  const QString pattern = qlua::getQString(L, 2);
  const QString text = qlua::getQString(L, 3);
  const QFlags<TriggerFlag> flags = (QFlags<TriggerFlag>)qlua::getInt(L, 4);
  const QColor color = qlua::getCustomColor(L, 5);
  // const int wildcardIndex = qlua::getInt(L, 6);
  const QString soundFile = qlua::getQString(L, 7);
  const optional<QString> script = qlua::getScriptName(L, 8);
  const optional<SendTarget> target = qlua::getSendTarget(L, 9, SendTarget::World);
  const int sequence = qlua::getInt(L, 10, 100);

  if (!script)
    return returnCode(L, ApiCode::ScriptNameNotLocated);

  if (!target)
    return returnCode(L, ApiCode::TriggerSendToInvalid);

  return returnCode(
      L,
      getApi(L).AddTrigger(
          getPluginIndex(L),
          name, pattern, text, flags, color, soundFile, *script, *target, sequence));
}

static int L_DeleteAlias(lua_State *L)
{
  API("DeleteAlias")
  expectMaxArgs(L, 1);
  return returnCode(L, getApi(L).DeleteAlias(getPluginIndex(L), qlua::getQString(L, 1)));
}

static int L_DeleteAliases(lua_State *L)
{
  API("DeleteAliases")
  expectMaxArgs(L, 1);
  lua_pushinteger(L, getApi(L).DeleteAliases(getPluginIndex(L), qlua::getQString(L, 1)));
  return 1;
}

static int L_DeleteTimer(lua_State *L)
{
  API("DeleteTimer")
  expectMaxArgs(L, 1);
  return returnCode(L, getApi(L).DeleteTimer(getPluginIndex(L), qlua::getQString(L, 1)));
}

static int L_DeleteTimers(lua_State *L)
{
  API("DeleteTimers")
  expectMaxArgs(L, 1);
  lua_pushinteger(L, getApi(L).DeleteTimers(getPluginIndex(L), qlua::getQString(L, 1)));
  return 1;
}

static int L_DeleteTrigger(lua_State *L)
{
  API("DeleteTrigger")
  expectMaxArgs(L, 1);
  return returnCode(L, getApi(L).DeleteTrigger(getPluginIndex(L), qlua::getQString(L, 1)));
}

static int L_DeleteTriggers(lua_State *L)
{
  API("DeleteTriggers")
  expectMaxArgs(L, 1);
  lua_pushinteger(L, getApi(L).DeleteTriggers(getPluginIndex(L), qlua::getQString(L, 1)));
  return 1;
}

static int L_DoAfter(lua_State *L)
{
  API("DoAfter")
  expectMaxArgs(L, 2);
  const double seconds = qlua::getNumber(L, 1);
  const QString text = qlua::getQString(L, 2);
  return returnCode(L, getApi(L).DoAfter(getPluginIndex(L), seconds, text, SendTarget::Command));
}

static int L_DoAfterNote(lua_State *L)
{
  API("DoAfterNote")
  expectMaxArgs(L, 2);
  const double seconds = qlua::getNumber(L, 1);
  const QString text = qlua::getQString(L, 2);
  return returnCode(L, getApi(L).DoAfter(getPluginIndex(L), seconds, text, SendTarget::Output));
}

static int L_DoAfterSpecial(lua_State *L)
{
  API("DoAfterSpecial")
  expectMaxArgs(L, 3);
  const double seconds = qlua::getNumber(L, 1);
  const QString text = qlua::getQString(L, 2);
  const optional<SendTarget> target = qlua::getSendTarget(L, 3);
  if (!target) [[unlikely]]
    return returnCode(L, ApiCode::OptionOutOfRange);
  return returnCode(L, getApi(L).DoAfter(getPluginIndex(L), seconds, text, *target));
}

static int L_DoAfterSpeedwalk(lua_State *L)
{
  API("DoAfterSpeedwalk")
  expectMaxArgs(L, 2);
  const double seconds = qlua::getNumber(L, 1);
  const QString text = qlua::getQString(L, 2);
  return returnCode(L, getApi(L).DoAfter(getPluginIndex(L), seconds, text, SendTarget::Speedwalk));
}

static int L_EnableAlias(lua_State *L)
{
  API("EnableAlias")
  expectMaxArgs(L, 2);
  return returnCode(
      L,
      getApi(L).EnableAlias(getPluginIndex(L), qlua::getQString(L, 1), qlua::getBool(L, 2, true)));
}

static int L_EnableAliasGroup(lua_State *L)
{
  API("EnableAliasGroup")
  expectMaxArgs(L, 2);
  return returnCode(
      L,
      getApi(L).EnableAliasGroup(getPluginIndex(L), qlua::getQString(L, 1), qlua::getBool(L, 2, true)));
}

static int L_EnableTimer(lua_State *L)
{
  API("EnableTimer")
  expectMaxArgs(L, 2);
  return returnCode(
      L,
      getApi(L).EnableTimer(getPluginIndex(L), qlua::getQString(L, 1), qlua::getBool(L, 2, true)));
}

static int L_EnableTimerGroup(lua_State *L)
{
  API("EnableTimerGroup")
  expectMaxArgs(L, 2);
  return returnCode(
      L,
      getApi(L).EnableTimerGroup(getPluginIndex(L), qlua::getQString(L, 1), qlua::getBool(L, 2, true)));
}

static int L_EnableTrigger(lua_State *L)
{
  API("EnableTrigger")
  expectMaxArgs(L, 2);
  return returnCode(
      L,
      getApi(L).EnableTrigger(getPluginIndex(L), qlua::getQString(L, 1), qlua::getBool(L, 2, true)));
}

static int L_EnableTriggerGroup(lua_State *L)
{
  API("EnableTriggerGroup")
  expectMaxArgs(L, 2);
  return returnCode(
      L,
      getApi(L).EnableTrigger(getPluginIndex(L), qlua::getQString(L, 1), qlua::getBool(L, 2, true)));
}

static int L_IsAlias(lua_State *L)
{
  API("IsAlias")
  expectMaxArgs(L, 1);
  return returnCode(L, getApi(L).IsAlias(getPluginIndex(L), qlua::getQString(L, 1)));
}

static int L_IsTimer(lua_State *L)
{
  API("IsTimer")
  expectMaxArgs(L, 1);
  return returnCode(L, getApi(L).IsTimer(getPluginIndex(L), qlua::getQString(L, 1)));
}

static int L_IsTrigger(lua_State *L)
{
  API("IsTrigger")
  expectMaxArgs(L, 1);
  return returnCode(L, getApi(L).IsTrigger(getPluginIndex(L), qlua::getQString(L, 1)));
}

static int L_SetTriggerOption(lua_State *L)
{
  API("SetTriggerOption")
  expectMaxArgs(L, 3);
  const QString label = qlua::getQString(L, 1);
  const string optionName = (string)qlua::getString(L, 2);
  const size_t plugin = getPluginIndex(L);
  if (const auto search = triggerBools.find(optionName); search != triggerBools.end())
    return returnCode(
        L,
        getApi(L).SetTriggerOption(plugin, label, search->second, qlua::getBool(L, 3)));

  if (optionName == "group")
    return returnCode(L, getApi(L).SetTriggerGroup(plugin, label, qlua::getQString(L, 3)));

  return returnCode(L, ApiCode::PluginCannotSetOption);
}

static int L_StopEvaluatingTriggers(lua_State *L)
{
  API("StopEvaluatingTriggers")
  expectMaxArgs(L, 1);
  getApi(L).StopEvaluatingTriggers();
  return 0;
}

// sound

static int L_PlaySound(lua_State *L)
{
  API("PlaySound")
  // long PlaySound(short Buffer, BSTR FileName, BOOL Loop, double Volume, double Pan);
  expectMaxArgs(L, 5);
  return returnCode(
      L,
      getApi(L).PlaySound(
          qlua::getInt(L, 1),
          qlua::getQString(L, 2),
          qlua::getBool(L, 3, false),
          convertVolume(qlua::getNumber(L, 4, 0.0))));
  // qlua::getDouble(L, 5) pan
}

static int L_PlaySoundMemory(lua_State *L)
{
  API("PlaySoundMemory")
  expectMaxArgs(L, 5);
  return returnCode(
      L,
      getApi(L).PlaySoundMemory(
          qlua::getInt(L, 1),
          qlua::getBytes(L, 2).toByteArray(),
          qlua::getBool(L, 3, false),
          convertVolume(qlua::getNumber(L, 4, 0.0))));
  // qlua::getDouble(L, 5) pan
}

static int L_StopSound(lua_State *L)
{
  API("StopSound")
  expectMaxArgs(L, 1);
  return returnCode(L, getApi(L).StopSound(qlua::getInt(L, 1)));
}

// variables

static int L_GetVariable(lua_State *L)
{
  API("GetVariable")
  expectMaxArgs(L, 1);
  const optional<string_view> var = getApi(L).GetVariable(getPluginIndex(L), qlua::getString(L, 1));
  if (!var)
    lua_pushnil(L);
  else
    qlua::pushString(L, *var);
  return 1;
}

static int L_GetPluginVariable(lua_State *L)
{
  API("GetPluginVariable")
  expectMaxArgs(L, 2);
  const optional<string_view> var = getApi(L).GetVariable(qlua::getString(L, 1), qlua::getString(L, 2));
  if (!var)
    lua_pushnil(L);
  else
    qlua::pushString(L, *var);
  return 1;
}

static int L_SetVariable(lua_State *L)
{
  API("SetVariable")
  expectMaxArgs(L, 2);
  getApi(L).SetVariable(getPluginIndex(L), qlua::getString(L, 1), qlua::getString(L, 2));
  return returnCode(L, ApiCode::OK);
}

// windows

static int L_Redraw(lua_State *L)
{
  API("Redraw")
  expectMaxArgs(L, 0);
  return 0;
}

static int L_Repaint(lua_State *L)
{
  API("Repaint")
  expectMaxArgs(L, 0);
  return 0;
}

static int L_TextRectangle(lua_State *L)
{
  API("TextRectangle")
  expectMaxArgs(L, 9);
  const QRect rect = qlua::getQRect(L, 1, 2, 3, 4);
  const int offset = qlua::getInt(L, 5);
  const QColor borderColor = qlua::getQColor(L, 6);
  const int borderWidth = qlua::getInt(L, 7);
  const QColor outsideColor = qlua::getQColor(L, 8);
  const optional<Qt::BrushStyle> outsideFillStyle = qlua::getBrush(L, 9);
  if (!outsideFillStyle) [[unlikely]]
    return returnCode(L, ApiCode::BrushStyleNotValid);
  return returnCode(
      L,
      getApi(L).TextRectangle(
          rect,
          offset,
          borderColor,
          borderWidth,
          QBrush(outsideColor, *outsideFillStyle)));
}

static int L_WindowCircleOp(lua_State *L)
{
  API("WindowCircleOp")
  expectMaxArgs(L, 15);
  const string_view windowName = qlua::getString(L, 1);
  const optional<CircleOp> action = qlua::getCircleOp(L, 2);
  const QRectF rect = qlua::getQRectF(L, 3, 4, 5, 6);
  const optional<QPen> pen = qlua::getPen(L, 7, 8, 9);
  const QColor brushColor = qlua::getQColor(L, 10);
  const optional<Qt::BrushStyle> brushStyle = qlua::getBrush(L, 11, Qt::BrushStyle::SolidPattern);
  if (!action) [[unlikely]]
    return returnCode(L, ApiCode::UnknownOption);
  if (!pen) [[unlikely]]
    return returnCode(L, ApiCode::PenStyleNotValid);
  if (!brushStyle) [[unlikely]]
    return returnCode(L, ApiCode::BrushStyleNotValid);
  const QBrush brush(brushColor, *brushStyle);

  switch (*action)
  {
  case CircleOp::Ellipse:
    return returnCode(L, getApi(L).WindowEllipse(windowName, rect, *pen, brush));
  case CircleOp::Rectangle:
    return returnCode(L, getApi(L).WindowRect(windowName, rect, *pen, brush));
  case CircleOp::RoundedRectangle:
    return returnCode(
        L,
        getApi(L).WindowRoundedRect(
            windowName,
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

static int L_WindowCreate(lua_State *L)
{
  API("WindowCreate")
  expectMaxArgs(L, 8);
  const string_view windowName = qlua::getString(L, 1);
  const QPoint location = qlua::getQPoint(L, 2, 3);
  const QSize size = qlua::getQSize(L, 4, 5);
  const optional<MiniWindow::Position> position = qlua::getWindowPosition(L, 6);
  const MiniWindow::Flags flags = (MiniWindow::Flags)qlua::getInt(L, 7);
  const QColor bg = qlua::getQColor(L, 8);
  if (!position) [[unlikely]]
    return returnCode(L, ApiCode::BadParameter);
  return returnCode(
      L,
      getApi(L).WindowCreate(
          getPluginIndex(L),
          windowName,
          location,
          size,
          *position,
          flags,
          bg));
}

static int L_WindowDrawImage(lua_State *L)
{
  API("WindowDrawImage")
  const int n = expectMaxArgs(L, 11);
  const string_view windowName = qlua::getString(L, 1);
  const string_view imageID = qlua::getString(L, 2);
  const QRectF rect = qlua::getQRectF(L, 3, 4, 5, 6);
  const optional<MiniWindow::DrawImageMode> mode =
      qlua::getDrawImageMode(L, 7, MiniWindow::DrawImageMode::Copy);
  const QRectF sourceRect = n >= 8 ? qlua::getQRectF(L, 8, 9, 10, 11) : QRectF();
  if (!mode)
    return returnCode(L, ApiCode::BadParameter);
  return returnCode(
      L,
      getApi(L).WindowDrawImage(windowName, imageID, rect, *mode, sourceRect));
}

static int L_WindowDrawImageAlpha(lua_State *L)
{
  API("WindowDrawImageAlpha")
  const int n = expectMaxArgs(L, 9);
  const string_view windowName = qlua::getString(L, 1);
  const string_view imageID = qlua::getString(L, 2);
  const QRectF rect = qlua::getQRectF(L, 3, 4, 5, 6);
  const double opacity = qlua::getNumber(L, 7);
  const QPointF origin = n >= 8 ? qlua::getQPointF(L, 8, 9) : QPointF();
  return returnCode(
      L,
      getApi(L).WindowDrawImageAlpha(windowName, imageID, rect, opacity, origin));
}

static int L_WindowEllipse(lua_State *L)
{
  API("WindowEllipse")
  expectMaxArgs(L, 15);
  const string_view windowName = qlua::getString(L, 1);
  const QRectF rect = qlua::getQRectF(L, 2, 3, 4, 5);
  const optional<QPen> pen = qlua::getPen(L, 6, 7, 8);
  const QColor fill = qlua::getQColor(L, 9);
  const optional<Qt::BrushStyle> brush = qlua::getBrush(L, 9, Qt::BrushStyle::SolidPattern);
  if (!pen) [[unlikely]]
    return returnCode(L, ApiCode::PenStyleNotValid);
  if (!brush) [[unlikely]]
    return returnCode(L, ApiCode::BrushStyleNotValid);
  return returnCode(L, getApi(L).WindowEllipse(windowName, rect, *pen, QBrush(fill, *brush)));
}

static int L_WindowFilter(lua_State *L)
{
  API("WindowFilter")
  expectMaxArgs(L, 7);
  using Channel = ImageFilter::Channel;
  const ScriptApi &api = getApi(L);
  const string_view windowName = qlua::getString(L, 1);
  const QRect rect = qlua::getQRect(L, 2, 3, 4, 5);
  switch (qlua::getInt(L, 6))
  {
  case 7:
    return returnCode(
        L,
        api.WindowFilter(windowName, BrightnessAddFilter(qlua::getInt(L, 7)), rect));
  case 10:
    return returnCode(
        L,
        api.WindowFilter(windowName, BrightnessAddFilter(qlua::getInt(L, 7), Channel::Red), rect));
  case 13:
    return returnCode(
        L,
        api.WindowFilter(windowName, BrightnessAddFilter(qlua::getInt(L, 7), Channel::Green), rect));
  case 16:
    return returnCode(
        L,
        api.WindowFilter(windowName, BrightnessAddFilter(qlua::getInt(L, 7), Channel::Blue), rect));
  case 19:
    return returnCode(L, api.WindowFilter(windowName, GrayscaleFilter(), rect));
  case 21:
    return returnCode(
        L,
        api.WindowFilter(windowName, BrightnessMultFilter(qlua::getNumber(L, 7)), rect));
  case 22:
    return returnCode(
        L,
        api.WindowFilter(windowName, BrightnessMultFilter(qlua::getNumber(L, 7), Channel::Red), rect));
  case 23:
    return returnCode(
        L,
        api.WindowFilter(windowName, BrightnessMultFilter(qlua::getNumber(L, 7), Channel::Green), rect));
  case 24:
    return returnCode(
        L,
        api.WindowFilter(windowName, BrightnessMultFilter(qlua::getNumber(L, 7), Channel::Blue), rect));
  default:
    return returnCode(L, ApiCode::UnknownOption);
  }
}

static int L_WindowFont(lua_State *L)
{
  API("WindowFont")
  expectMaxArgs(L, 10);
  const string_view windowName = qlua::getString(L, 1);
  const string_view fontID = qlua::getString(L, 2);
  const QString fontName = qlua::getQString(L, 3);
  const double pointSize = qlua::getNumber(L, 4);
  if (pointSize == 0 && fontName.isEmpty()) [[unlikely]]
    return returnCode(L, getApi(L).WindowUnloadFont(windowName, fontID));
  const bool bold = qlua::getBool(L, 5, false);
  const bool italic = qlua::getBool(L, 6, false);
  const bool underline = qlua::getBool(L, 7, false);
  const bool strikeout = qlua::getBool(L, 8, false);
  // const short charset = qlua::getInt(L, 9);
  const optional<QFont::StyleHint> hint = qlua::getFontHint(L, 10, QFont::StyleHint::AnyStyle);
  if (!hint) [[unlikely]]
    return returnCode(L, ApiCode::BadParameter);
  return returnCode(
      L,
      getApi(L).WindowFont(
          windowName,
          fontID,
          fontName,
          pointSize,
          bold,
          italic,
          underline,
          strikeout,
          *hint));
}

static int L_WindowFontInfo(lua_State *L)
{
  API("WindowFontInfo")
  expectMaxArgs(L, 3);
  qlua::pushQVariant(
      L,
      getApi(L).WindowFontInfo(qlua::getString(L, 1), qlua::getString(L, 2), qlua::getInt(L, 3)));
  return 1;
}

static int L_WindowGradient(lua_State *L)
{
  API("WindowGradient")
  expectMaxArgs(L, 8);
  const string_view windowName = qlua::getString(L, 1);
  const QRectF rect = qlua::getQRectF(L, 2, 3, 4, 5);
  const QColor color1 = qlua::getQColor(L, 6);
  const QColor color2 = qlua::getQColor(L, 7);
  const int mode = qlua::getInt(L, 8);
  if (mode != Qt::Horizontal && mode != Qt::Vertical) [[unlikely]]
    return returnCode(L, ApiCode::UnknownOption);
  return returnCode(
      L,
      getApi(L).WindowGradient(
          windowName,
          rect,
          color1,
          color2,
          (Qt::Orientation)mode));
}

static int L_WindowImageFromWindow(lua_State *L)
{
  API("WindowImageFromWindow")
  expectMaxArgs(L, 3);
  const string_view windowName = qlua::getString(L, 1);
  const string_view imageID = qlua::getString(L, 2);
  const string_view sourceWindow = qlua::getString(L, 3);
  return returnCode(L, getApi(L).WindowImageFromWindow(windowName, imageID, sourceWindow));
}

static int L_WindowLine(lua_State *L)
{
  API("WindowLine")
  expectMaxArgs(L, 8);
  const string_view windowName = qlua::getString(L, 1);
  const QLineF line = qlua::getQLineF(L, 2, 3, 4, 5);
  const optional<QPen> pen = qlua::getPen(L, 6, 7, 8);
  if (!pen) [[unlikely]]
    return returnCode(L, ApiCode::PenStyleNotValid);
  return returnCode(L, getApi(L).WindowLine(windowName, line, *pen));
}

static int L_WindowLoadImage(lua_State *L)
{
  API("WindowLoadImage")
  expectMaxArgs(L, 3);
  const string_view windowName = qlua::getString(L, 1);
  const string_view imageID = qlua::getString(L, 2);
  const QString filename = qlua::getQString(L, 3);
  if (filename.isEmpty()) [[unlikely]]
    return returnCode(L, getApi(L).WindowUnloadImage(windowName, imageID));
  return returnCode(L, getApi(L).WindowLoadImage(windowName, imageID, filename));
}

static int L_WindowMenu(lua_State *L)
{
  API("WindowMenu")
  expectMaxArgs(L, 4);
  const QVariant result = getApi(L).WindowMenu(
      qlua::getString(L, 1),
      qlua::getQPoint(L, 2, 3),
      qlua::getString(L, 4));
  qlua::pushQVariant(L, result);
  return 1;
}

static int L_WindowPolygon(lua_State *L)
{
  API("WindowPolygon")
  expectMaxArgs(L, 9);
  const string_view windowName = qlua::getString(L, 1);
  const optional<QPolygonF> polygon = qlua::getQPolygonF(L, 2);
  const optional<QPen> pen = qlua::getPen(L, 3, 4, 5);
  const QColor brushColor = qlua::getQColor(L, 6);
  const optional<Qt::BrushStyle> brushStyle = qlua::getBrush(L, 7);
  const bool close = qlua::getBool(L, 8, false);
  const bool winding = qlua::getBool(L, 9, false);
  if (!polygon) [[unlikely]]
    return returnCode(L, ApiCode::InvalidNumberOfPoints);
  if (!pen) [[unlikely]]
    return returnCode(L, ApiCode::PenStyleNotValid);
  if (!brushStyle) [[unlikely]]
    return returnCode(L, ApiCode::BrushStyleNotValid);
  return returnCode(
      L,
      getApi(L).WindowPolygon(
          windowName,
          *polygon,
          *pen,
          QBrush(brushColor, *brushStyle),
          close,
          winding ? Qt::FillRule::WindingFill : Qt::FillRule::OddEvenFill));
}

static int L_WindowPosition(lua_State *L)
{
  API("WindowPosition")
  expectMaxArgs(L, 5);
  const string_view windowName = qlua::getString(L, 1);
  const QPoint location = qlua::getQPoint(L, 2, 3);
  const optional<MiniWindow::Position> position = qlua::getWindowPosition(L, 4);
  const MiniWindow::Flags flags = (MiniWindow::Flags)qlua::getInt(L, 5);
  if (!position) [[unlikely]]
    return returnCode(L, ApiCode::BadParameter);
  return returnCode(L, getApi(L).WindowPosition(windowName, location, *position, flags));
}

static int L_WindowRectOp(lua_State *L)
{
  API("WindowRectOp")
  expectMaxArgs(L, 8);
  const string_view windowName = qlua::getString(L, 1);
  const optional<RectOp> action = qlua::getRectOp(L, 2);
  const QRectF rect = qlua::getQRectF(L, 3, 4, 5, 6);
  if (!action) [[unlikely]]
    return returnCode(L, ApiCode::UnknownOption);

  switch (*action)
  {
  case RectOp::Frame:
    return returnCode(L, getApi(L).WindowRect(windowName, rect, qlua::getQColor(L, 7), QBrush()));
  case RectOp::Fill:
    return returnCode(L, getApi(L).WindowRect(windowName, rect, QPen(), qlua::getQColor(L, 7)));
  case RectOp::Invert:
    return returnCode(L, getApi(L).WindowInvert(windowName, rect.toRect()));
  case RectOp::Frame3D:
    return returnCode(
        L,
        getApi(L).WindowFrame(
            windowName,
            rect,
            qlua::getQColor(L, 7),
            qlua::getQColor(L, 8, Qt::GlobalColor::black)));
  case RectOp::Edge3D:
  case RectOp::FloodFillBorder:
  case RectOp::FloodFillSurface:
    return returnCode(L, ApiCode::OK);
  }
}

static int L_WindowResize(lua_State *L)
{
  API("WindowResize")
  expectMaxArgs(L, 4);
  return returnCode(
      L,
      getApi(L).WindowResize(qlua::getString(L, 1), qlua::getQSize(L, 2, 3), qlua::getQColor(L, 4)));
}

static int L_WindowSetZOrder(lua_State *L)
{
  API("WindowSetZOrder")
  expectMaxArgs(L, 2);
  return returnCode(L, getApi(L).WindowSetZOrder(qlua::getString(L, 1), qlua::getInt(L, 2)));
}

static int L_WindowShow(lua_State *L)
{
  API("WindowShow")
  expectMaxArgs(L, 2);
  return returnCode(L, getApi(L).WindowShow(qlua::getString(L, 1), qlua::getBool(L, 2, true)));
}

static int L_WindowText(lua_State *L)
{
  API("WindowText")
  expectMaxArgs(L, 9);
  const string_view windowName = qlua::getString(L, 1);
  const string_view fontID = qlua::getString(L, 2);
  const string_view view = qlua::toString(L, 3);
  const QRectF rect = qlua::getQRectF(L, 4, 5, 6, 7);
  const QColor color = qlua::getQColor(L, 8);
  const bool unicode = qlua::getBool(L, 9, false);
  const QString text = unicode ? QString::fromUtf8(view.data(), view.size())
                               : QString::fromLatin1(view.data(), view.size());
  const qreal width = getApi(L).WindowText(windowName, fontID, text, rect, color);
  lua_pushinteger(L, width);
  return 1;
}

static int L_WindowTextWidth(lua_State *L)
{
  API("WindowTextWidth")
  expectMaxArgs(L, 4);
  const string_view windowName = qlua::getString(L, 1);
  const string_view fontID = qlua::getString(L, 2);
  const string_view view = qlua::getString(L, 3);
  const bool unicode = qlua::getBool(L, 4, false);
  const QString text = unicode ? QString::fromUtf8(view.data(), view.size())
                               : QString::fromLatin1(view.data(), view.size());
  const int width = getApi(L).WindowTextWidth(windowName, fontID, text);
  lua_pushinteger(L, width);
  return 1;
}

// window hotspots

static int L_WindowAddHotspot(lua_State *L)
{
  API("WindowAddHotspot")
  expectMaxArgs(L, 14);
  const string_view windowName = qlua::getString(L, 1);
  const string_view hotspotID = qlua::getString(L, 2);
  const QRect geometry(qlua::getQPoint(L, 3, 4), qlua::getQPoint(L, 5, 6));
  Hotspot::Callbacks callbacks{
      .dragMove = "",
      .dragRelease = "",
      .mouseOver = (string)qlua::getString(L, 7, ""),
      .cancelMouseOver = (string)qlua::getString(L, 8, ""),
      .mouseDown = (string)qlua::getString(L, 9, ""),
      .cancelMouseDown = (string)qlua::getString(L, 10, ""),
      .mouseUp = (string)qlua::getString(L, 11, ""),
      .scroll = "",
  };
  const QString &tooltip = qlua::getQString(L, 12, QString());
  const optional<Qt::CursorShape> cursor = qlua::getCursor(L, 13, Qt::CursorShape::ArrowCursor);
  const Hotspot::Flags flags = (Hotspot::Flags)qlua::getInt(L, 14, 0);
  if (!cursor) [[unlikely]]
    return returnCode(L, ApiCode::BadParameter);
  return returnCode(
      L,
      getApi(L).WindowAddHotspot(
          getPluginIndex(L),
          windowName,
          hotspotID,
          geometry,
          std::move(callbacks),
          tooltip,
          *cursor,
          flags));
}

static int L_WindowDeleteHotspot(lua_State *L)
{
  API("WindowDeleteHotspot")
  expectMaxArgs(L, 2);
  return returnCode(L, getApi(L).WindowDeleteHotspot(qlua::getString(L, 1), qlua::getString(L, 2)));
}

static int L_WindowDragHandler(lua_State *L)
{
  API("WindowDragHandler")
  expectMaxArgs(L, 5);
  return returnCode(
      L,
      getApi(L).WindowUpdateHotspot(
          getPluginIndex(L),
          qlua::getString(L, 1),
          qlua::getString(L, 2),
          Hotspot::CallbacksPartial{
              .dragMove = (string)qlua::getString(L, 3, ""),
              .dragRelease = (string)qlua::getString(L, 4, ""),
              .mouseOver = nullopt,
              .cancelMouseOver = nullopt,
              .mouseDown = nullopt,
              .cancelMouseDown = nullopt,
              .mouseUp = nullopt,
              .scroll = nullopt}));
}

static int L_WindowMoveHotspot(lua_State *L)
{
  API("WindowMoveHotspot")
  expectMaxArgs(L, 6);
  return returnCode(
      L,
      getApi(L).WindowMoveHotspot(
          qlua::getString(L, 1),
          qlua::getString(L, 2),
          QRect(qlua::getQPoint(L, 3, 4), qlua::getQPoint(L, 5, 6))));
}

static int L_WindowScrollwheelHandler(lua_State *L)
{
  API("WindowScrollwheelHandler")
  expectMaxArgs(L, 3);
  return returnCode(
      L,
      getApi(L).WindowUpdateHotspot(
          getPluginIndex(L),
          qlua::getString(L, 1),
          qlua::getString(L, 2),
          Hotspot::CallbacksPartial{
              .dragMove = nullopt,
              .dragRelease = nullopt,
              .mouseOver = nullopt,
              .cancelMouseOver = nullopt,
              .mouseDown = nullopt,
              .cancelMouseDown = nullopt,
              .mouseUp = nullopt,
              .scroll = (string)qlua::getString(L, 3, "")}));
}

// userdata

static int L_noop(lua_State *L)
{
  API("noop")
  return returnCode(L, ApiCode::OK);
}

static const struct luaL_Reg worldlib[] =
    // database
    {{"DatabaseClose", L_DatabaseClose},
     {"DatabaseOpen", L_DatabaseOpen},
     // info
     {"GetInfo", L_GetInfo},
     {"GetLineInfo", L_GetLineInfo},
     {"GetStyleInfo", L_GetStyleInfo},
     {"Version", L_Version},
     {"WindowFontInfo", L_WindowFontInfo},
     {"WindowInfo", L_WindowInfo},
     // input
     {"Send", L_Send},
     {"SendNoEcho", L_SendNoEcho},
     {"SendPkt", L_SendPkt},
     // options
     {"GetAlphaOption", L_GetAlphaOption},
     {"GetAlphaOptionList", L_GetAlphaOptionList},
     {"GetCurrentValue", L_GetCurrentValue},
     {"GetOption", L_GetOption},
     {"GetOptionList", L_GetOptionList},
     {"SetAlphaOption", L_SetAlphaOption},
     {"SetOption", L_SetOption},
     // output
     {"ColourNameToRGB", L_ColourNameToRGB},
     {"ColourNote", L_ColourNote},
     {"ColourTell", L_ColourTell},
     {"GetLinesInBufferCount", L_GetLinesInBufferCount},
     {"Hyperlink", L_Hyperlink},
     {"Note", L_Note},
     {"PickColour", L_PickColour},
     {"RGBColourToName", L_RGBColourToName},
     {"SetClipboard", L_SetClipboard},
     {"SetCursor", L_SetCursor},
     {"SetStatus", L_SetStatus},
     {"Tell", L_Tell},
     // plugins
     {"BroadcastPlugin", L_BroadcastPlugin},
     {"CallPlugin", L_CallPlugin},
     {"EnablePlugin", L_EnablePlugin},
     {"GetPluginID", L_GetPluginID},
     {"GetPluginInfo", L_GetPluginInfo},
     {"PluginSupports", L_PluginSupports},
     // senders
     {"AddAlias", L_AddAlias},
     {"AddTimer", L_AddTimer},
     {"AddTrigger", L_AddTrigger},
     {"AddTriggerEx", L_AddTrigger},
     {"DeleteAlias", L_DeleteAlias},
     {"DeleteAliases", L_DeleteAliases},
     {"DeleteTimer", L_DeleteTimer},
     {"DeleteTimers", L_DeleteTimers},
     {"DeleteTrigger", L_DeleteTrigger},
     {"DeleteTriggers", L_DeleteTriggers},
     {"DoAfter", L_DoAfter},
     {"DoAfterNote", L_DoAfterNote},
     {"DoAfterSpeedwalk", L_DoAfterSpeedwalk},
     {"DoAfterSpecial", L_DoAfterSpecial},
     {"EnableAlias", L_EnableAlias},
     {"EnableAliasGroup", L_EnableAliasGroup},
     {"EnableTimer", L_EnableTimer},
     {"EnableTimerGroup", L_EnableTimerGroup},
     {"EnableTrigger", L_EnableTrigger},
     {"EnableTriggerGroup", L_EnableTriggerGroup},
     {"IsAlias", L_IsAlias},
     {"IsTimer", L_IsTimer},
     {"IsTrigger", L_IsTrigger},
     {"SetTriggerOption", L_SetTriggerOption},
     {"StopEvaluatingTriggers", L_StopEvaluatingTriggers},
     // sound
     {"PlaySound", L_PlaySound},
     {"PlaySoundMemory", L_PlaySoundMemory},
     {"StopSound", L_StopSound},
     // variables
     {"GetVariable", L_GetVariable},
     {"GetPluginVariable", L_GetPluginVariable},
     {"SetVariable", L_SetVariable},
     // windows
     {"Redraw", L_Redraw},
     {"Repaint", L_Repaint},
     {"TextRectangle", L_TextRectangle},
     {"WindowCircleOp", L_WindowCircleOp},
     {"WindowCreate", L_WindowCreate},
     {"WindowDrawImage", L_WindowDrawImage},
     {"WindowDrawImageAlpha", L_WindowDrawImageAlpha},
     {"WindowEllipse", L_WindowEllipse},
     {"WindowFilter", L_WindowFilter},
     {"WindowFont", L_WindowFont},
     {"WindowGradient", L_WindowGradient},
     {"WindowImageFromWindow", L_WindowImageFromWindow},
     {"WindowLine", L_WindowLine},
     {"WindowLoadImage", L_WindowLoadImage},
     {"WindowMenu", L_WindowMenu},
     {"WindowPolygon", L_WindowPolygon},
     {"WindowPosition", L_WindowPosition},
     {"WindowRectOp", L_WindowRectOp},
     {"WindowResize", L_WindowResize},
     {"WindowSetZOrder", L_WindowSetZOrder},
     {"WindowShow", L_WindowShow},
     {"WindowText", L_WindowText},
     {"WindowTextWidth", L_WindowTextWidth},
     // window hotspots
     {"WindowAddHotspot", L_WindowAddHotspot},
     {"WindowDeleteHotspot", L_WindowDeleteHotspot},
     {"WindowDragHandler", L_WindowDragHandler},
     {"WindowMoveHotspot", L_WindowMoveHotspot},
     {"WindowScrollwheelHandler", L_WindowScrollwheelHandler},
     // stubs
     {"SetBackgroundImage", L_noop},
     {"SetFrameBackgroundColour", L_noop},

     {NULL, NULL}};

static int L_world_tostring(lua_State *L)
{
  API("world_tostring")
  lua_pushliteral(L, "world");
  return 1;
}

static const struct luaL_Reg worldlib_meta[] = {{"__tostring", L_world_tostring}, {NULL, NULL}};

int registerLuaWorld(lua_State *L)
{
  luaL_newlib(L, worldlib);

  luaL_newmetatable(L, worldRegKey);
  luaL_setfuncs(L, worldlib_meta, 0);
  lua_setmetatable(L, -2);

  lua_setglobal(L, worldLibKey);

  lua_pushglobaltable(L);
  if (lua_getmetatable(L, -1) == LUA_TNIL)
    lua_newtable(L);

  lua_getglobal(L, worldLibKey);
  lua_setfield(L, -2, "__index");
  lua_setmetatable(L, -2);

  return 1;
}
