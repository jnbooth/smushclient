#include "luaapi.h"
#include <QtCore/QPointer>
#include "qlua.h"
#include "scriptapi.h"
#include "scriptenums.h"
#include "worldproperties.h"
extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#define API_REG_KEY "smushclient.api"
#define ID_REG_KEY "smushclient.pluginid"
#define VARIABLES_REG_KEY "smushclient.vars"
#define WORLD_LIB_KEY "world"
#define WORLD_REG_KEY "smushclient.world"

using std::string;
using std::string_view;
using stringmap = std::unordered_map<string, string>;

// localization

QString fmtNoSuchPlugin(const QString &id)
{
  return ScriptApi::tr("Plugin ID (%1) is not installed")
      .arg(id);
}

QString fmtPluginDisabled(const Plugin &plugin)
{
  return ScriptApi::tr("Plugin '%1' (%2) is not enabled")
      .arg(plugin.name())
      .arg(plugin.id());
}

QString fmtSelfCall(const Plugin &plugin)
{
  return ScriptApi::tr("Plugin '%1' (%2) attempted to call itself via CallPlugin")
      .arg(plugin.name())
      .arg(plugin.id());
}

QString fmtNoSuchRoutine(const Plugin &plugin, string_view routine)
{
  return ScriptApi::tr("No function '%1' in plugin '%2' (%3)")
      .arg(QString::fromUtf8(routine.data(), routine.size()))
      .arg(plugin.name())
      .arg(plugin.id());
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
      .arg(plugin.name())
      .arg(plugin.id());
}

QString fmtBadReturn(const Plugin &plugin, string_view routine, int idx, const char *type)
{
  return ScriptApi::tr("Cannot handle return value #%1 (%2 type) from function '%3' in plugin '%4' (%5)")
      .arg(idx)
      .arg(QString::fromUtf8(type))
      .arg(QString::fromUtf8(routine.data(), routine.size()))
      .arg(plugin.name())
      .arg(plugin.id());
}

// utils

inline int returnCode(lua_State *L, ApiCode code)
{
  lua_pushinteger(L, (lua_Integer)code);
  return 1;
}

inline int returnCode(lua_State *L, ApiCode code, const QString &reason)
{
  lua_pushinteger(L, (lua_Integer)code);
  qlua::pushQString(L, reason);
  return 2;
}

template <typename T>
inline T *createUserdata(lua_State *L, const char *k)
{
  void *ud = lua_newuserdata(L, sizeof(T));
  lua_setfield(L, LUA_REGISTRYINDEX, k);
  return static_cast<T *>(ud);
}

template <typename T>
inline T *getUserdata(lua_State *L, const char *k)
{
  lua_getfield(L, LUA_REGISTRYINDEX, k);
  void *ud = lua_touserdata(L, -1);
  lua_pop(L, 1);
  return static_cast<T *>(ud);
}

void returnError(lua_State *L, const char *e)
{
  lua_pushstring(L, e);
  lua_error(L);
}

inline ScriptApi &getApi(lua_State *L)
{
  QPointer<ScriptApi> *ud = getUserdata<QPointer<ScriptApi>>(L, API_REG_KEY);
  if (ud == nullptr || ud->isNull())
    returnError(L, "Userdata was deleted");
  return *ud->data();
}

void setLuaApi(lua_State *L, ScriptApi *api)
{
  *createUserdata<QPointer<ScriptApi>>(L, API_REG_KEY) = api;
}

void setPluginID(lua_State *L, std::string_view pluginID)
{
  qlua::pushString(L, pluginID);
  lua_setfield(L, LUA_REGISTRYINDEX, ID_REG_KEY);
}

// options

static int L_GetAlphaOption(lua_State *L)
{
  const QVariant option = getApi(L).GetOption(qlua::getString(L, 1));
  QString value = option.toString();
  if (value.isEmpty() && option.canConvert<QColor>())
    value = option.value<QColor>().name();
  qlua::pushQString(L, value);
  return 1;
}

static int L_GetAlphaOptionList(lua_State *L)
{
  qlua::pushStrings(L, WorldProperties::stringOptions());
  return 1;
}

static int L_GetCurrentValue(lua_State *L)
{
  qlua::pushQVariant(L, getApi(L).GetOption(qlua::getString(L, 1)));
  return 1;
}

static int L_GetOption(lua_State *L)
{
  const QVariant option = getApi(L).GetOption(qlua::getString(L, 1));
  const QMetaType type = option.metaType();
  switch (type.id())
  {
  case QMetaType::Double:
  case QMetaType::Float:
  case QMetaType::Float16:
    lua_pushnumber(L, option.value<lua_Number>());
    break;
  case QMetaType::QColor:
    lua_pushinteger(L, qlua::colorToRgbCode(option.value<QColor>()));
    break;
  default:
    if (option.canConvert<lua_Integer>())
      lua_pushinteger(L, option.value<lua_Integer>());
    else
      lua_pushinteger(L, -1);
  }
  return 1;
}

static int L_GetOptionList(lua_State *L)
{
  qlua::pushStrings(L, WorldProperties::numericOptions());
  return 1;
}

static int L_SetAlphaOption(lua_State *L)
{
  getApi(L).SetOption(qlua::getString(L, 1), qlua::getQString(L, 2));
  return 1;
}

static int L_SetOption(lua_State *L)
{
  getApi(L).SetOption(qlua::getString(L, 1), qlua::getNumber(L, 2));
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

inline void insertTexts(lua_State *L, ScriptApi &api)
{
  int n = lua_gettop(L);
  for (int i = 1; i <= n; ++i)
    api.Tell(qlua::getQString(L, i));
}

static int L_ColourNameToRGB(lua_State *L)
{
  lua_pushinteger(L, qlua::colorToRgbCode(qlua::getQColor(L, 1)));
  return 1;
}

static int L_ColourNote(lua_State *L)
{
  ScriptApi &api = getApi(L);
  insertTextTriples(L, api);
  api.finishNote();
  return 0;
}

static int L_ColourTell(lua_State *L)
{
  insertTextTriples(L, getApi(L));
  return 0;
}

static int L_Hyperlink(lua_State *L)
{
  getApi(L).Hyperlink(
      qlua::getQString(L, 1),
      qlua::getQString(L, 2),
      qlua::getQString(L, 3),
      qlua::getQColor(L, 4),
      qlua::getQColor(L, 5),
      qlua::getBool(L, 6),
      qlua::getBool(L, 7, false));
  return 0;
}

static int L_Note(lua_State *L)
{
  ScriptApi &api = getApi(L);
  insertTexts(L, api);
  api.finishNote();
  return 0;
}

static int L_RGBColourToName(lua_State *L)
{
  qlua::pushQColor(L, qlua::rgbCodeToColor(qlua::getInt(L, 1)));
  return 1;
}

static int L_Send(lua_State *L)
{
  return returnCode(L, getApi(L).Send(qlua::borrowBytes(L, 1)));
}

static int L_SendNoEcho(lua_State *L)
{
  return returnCode(L, getApi(L).SendNoEcho(qlua::borrowBytes(L, 1)));
}

static int L_SetClipboard(lua_State *L)
{
  ScriptApi::SetClipboard(qlua::getQString(L, 1));
  return 0;
}

static int L_Tell(lua_State *L)
{
  insertTexts(L, getApi(L));
  return 0;
}

// plugins

static int L_CallPlugin(lua_State *L)
{
  const Plugin *pluginRef = getApi(L).getPlugin(qlua::getString(L, 1));
  if (pluginRef == nullptr)
    return returnCode(
        L,
        ApiCode::NoSuchPlugin,
        fmtNoSuchPlugin(qlua::getQString(L, 1)));

  const Plugin &plugin = *pluginRef;
  if (plugin.disabled())
    return returnCode(L, ApiCode::PluginDisabled, fmtPluginDisabled(plugin));

  const string_view routine = qlua::getString(L, 2);

  lua_State *L2 = plugin.state();
  if (L2 == L)
    return returnCode(
        L,
        ApiCode::ErrorCallingPluginRoutine, fmtSelfCall(plugin));

  const int n = lua_gettop(L);
  luaL_checkstack(L2, n - 1, nullptr);

  if (lua_getglobal(L2, routine.data()) != LUA_TFUNCTION)
    return returnCode(L, ApiCode::NoSuchRoutine, fmtNoSuchRoutine(plugin, routine));

  const int topBefore = lua_gettop(L2) - 1;

  for (int i = 3; i <= n; ++i)
    if (!qlua::copyValue(L, L2, i))
    {
      lua_settop(L2, topBefore);
      lua_settop(L, 0);
      return returnCode(L, ApiCode::BadParameter, fmtBadParam(i - 2, luaL_typename(L, i)));
    }

  if (lua_pcall(L2, n, LUA_MULTRET, 0) != LUA_OK)
  {
    lua_settop(L, 0);
    lua_pushinteger(L, (lua_Integer)ApiCode::ErrorCallingPluginRoutine);
    qlua::pushQString(L, fmtCallError(plugin, routine));
    size_t size;
    lua_pushlstring(L, lua_tolstring(L2, -1, &size), size);
    lua_settop(L2, topBefore);
    return 3;
  }

  const int topAfter = lua_gettop(L2);
  const int nresults = topAfter - topBefore + 1;
  lua_settop(L, 0);
  luaL_checkstack(L, nresults, nullptr);
  lua_pushinteger(L, (lua_Integer)ApiCode::OK);
  for (int i = topBefore + 1; i <= topAfter; ++i)
  {
    if (!qlua::copyValue(L, L2, i))
      return returnCode(L, ApiCode::ErrorCallingPluginRoutine,
                        fmtBadReturn(plugin, routine, i - topBefore, luaL_typename(L, i)));
  }
  lua_settop(L2, topBefore);
  return nresults;
}

static int L_GetPluginInfo(lua_State *L)
{
  const string_view pluginID = qlua::getString(L, 1);
  const lua_Integer infoType = qlua::getInt(L, 2);
  if (infoType > UINT8_MAX)
  {
    lua_pushnil(L);
    return 1;
  }
  qlua::pushQVariant(L, getApi(L).GetPluginInfo(pluginID, (uint8_t)infoType));
  return 1;
}

// senders

static int L_EnableAlias(lua_State *L)
{
  return returnCode(L, getApi(L).EnableAlias(qlua::getQString(L, 1), qlua::getBool(L, 2)));
}

static int L_EnableAliasGroup(lua_State *L)
{
  return returnCode(L, getApi(L).EnableAliasGroup(qlua::getQString(L, 1), qlua::getBool(L, 2)));
}

static int L_EnablePlugin(lua_State *L)
{
  return returnCode(L, getApi(L).EnablePlugin(qlua::getString(L, 1), qlua::getBool(L, 2)));
}

static int L_EnableTimer(lua_State *L)
{
  return returnCode(L, getApi(L).EnableTimer(qlua::getQString(L, 1), qlua::getBool(L, 2)));
}

static int L_EnableTimerGroup(lua_State *L)
{
  return returnCode(L, getApi(L).EnableTimerGroup(qlua::getQString(L, 1), qlua::getBool(L, 2)));
}

static int L_EnableTrigger(lua_State *L)
{
  return returnCode(L, getApi(L).EnableTrigger(qlua::getQString(L, 1), qlua::getBool(L, 2)));
}

static int L_EnableTriggerGroup(lua_State *L)
{
  return returnCode(L, getApi(L).EnableTrigger(qlua::getQString(L, 1), qlua::getBool(L, 2)));
}

static int L_IsAlias(lua_State *L)
{
  return returnCode(L, getApi(L).IsAlias(qlua::getQString(L, 1)));
}

static int L_IsTimer(lua_State *L)
{
  return returnCode(L, getApi(L).IsTimer(qlua::getQString(L, 1)));
}

static int L_IsTrigger(lua_State *L)
{
  return returnCode(L, getApi(L).IsTrigger(qlua::getQString(L, 1)));
}

// variables

stringmap *createVariableMap(lua_State *L)
{
  stringmap *vars = createUserdata<stringmap>(L, VARIABLES_REG_KEY);
  *vars = stringmap();
  return vars;
}

inline stringmap *getVariableMap(lua_State *L)
{
  return getUserdata<stringmap>(L, VARIABLES_REG_KEY);
}

inline void pushVariable(lua_State *L, const stringmap &vars, string_view name)
{
  if (auto search = vars.find((string)name); search != vars.end())
    qlua::pushString(L, search->second);
  else
    lua_pushnil(L);
}

static int L_GetVariable(lua_State *L)
{
  pushVariable(L, *getVariableMap(L), qlua::getString(L, 1));
  return 1;
}

static int L_GetPluginVariable(lua_State *L)
{
  const Plugin *plugin = getApi(L).getPlugin(qlua::getString(L, 1));
  if (plugin == nullptr)
    lua_pushnil(L);
  else
    pushVariable(L, *plugin->variables(), qlua::getString(L, 2));
  return 1;
}

static int L_SetVariable(lua_State *L)
{
  (*getVariableMap(L))[(string)qlua::getString(L, 1)] = qlua::getString(L, 2);
  return returnCode(L, ApiCode::OK);
}

// windows

static int L_Redraw(lua_State *)
{
  return 0;
}

static int L_Repaint(lua_State *)
{
  return 0;
}

static int L_WindowCreate(lua_State *L)
{
  return returnCode(
      L,
      getApi(L).WindowCreate(
          qlua::getString(L, 1),
          QPoint(qlua::getInt(L, 2), qlua::getInt(L, 3)),
          QSize(qlua::getInt(L, 4), qlua::getInt(L, 5)),
          qlua::getWindowPosition(L, 6),
          (MiniWindow::Flags)(int)qlua::getInt(L, 7),
          qlua::getQColor(L, 8)));
}

static int L_WindowPosition(lua_State *L)
{
  return returnCode(
      L,
      getApi(L).WindowPosition(
          qlua::getString(L, 1),
          QPoint(qlua::getInt(L, 2), qlua::getInt(L, 3)),
          qlua::getWindowPosition(L, 6),
          (MiniWindow::Flags)(int)qlua::getInt(L, 7)));
}

static int L_WindowResize(lua_State *L)
{
  return returnCode(
      L,
      getApi(L).WindowResize(
          qlua::getString(L, 1),
          QSize(qlua::getInt(L, 2), qlua::getInt(L, 3)),
          qlua::getQColor(L, 4)));
}

static int L_WindowSetZOrder(lua_State *L)
{
  return returnCode(L, getApi(L).WindowSetZOrder(qlua::getString(L, 1), qlua::getInt(L, 2)));
}

static int L_WindowShow(lua_State *L)
{
  return returnCode(L, getApi(L).WindowShow(qlua::getString(L, 1), qlua::getBool(L, 2)));
}

// window hotspots

static int L_WindowAddHotspot(lua_State *L)
{
  return returnCode(
      L,
      getApi(L).WindowAddHotspot(
          qlua::getString(L, 1),
          qlua::getString(L, 2),
          qlua::getString(L, 3),
          QRect(
              QPoint(qlua::getInt(L, 4), qlua::getInt(L, 5)),
              QPoint(qlua::getInt(L, 6), qlua::getInt(L, 7))),
          Hotspot::Callbacks{
              .mouseOver = (string)qlua::getString(L, 8),
              .cancelMouseOver = (string)qlua::getString(L, 9),
              .mouseDown = (string)qlua::getString(L, 10),
              .cancelMouseDown = (string)qlua::getString(L, 11),
              .mouseUp = (string)qlua::getString(L, 12),
          },
          qlua::getQString(L, 13),
          qlua::getCursor(L, 14),
          qlua::getInt(L, 15) & 0x01));
}

static int L_WindowDeleteHotspot(lua_State *L)
{
  return returnCode(L, getApi(L).WindowDeleteHotspot(qlua::getString(L, 1), qlua::getString(L, 2)));
}

static int L_WindowMoveHotspot(lua_State *L)
{
  return returnCode(
      L,
      getApi(L).WindowMoveHotspot(
          qlua::getString(L, 1),
          qlua::getString(L, 2),
          QRect(
              QPoint(qlua::getInt(L, 3), qlua::getInt(L, 4)),
              QPoint(qlua::getInt(L, 5), qlua::getInt(L, 6)))));
}

// userdata

static const struct luaL_Reg worldlib[] =
    // options
    {{"GetAlphaOption", L_GetAlphaOption},
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
     {"Hyperlink", L_Hyperlink},
     {"Note", L_Note},
     {"RGBColourToName", L_RGBColourToName},
     {"Send", L_Send},
     {"SendNoEcho", L_SendNoEcho},
     {"SetClipboard", L_SetClipboard},
     {"Tell", L_Tell},
     // plugins
     {"CallPlugin", L_CallPlugin},
     {"GetPluginInfo", L_GetPluginInfo},
     // senders
     {"EnableAlias", L_EnableAlias},
     {"EnableAliasGroup", L_EnableAliasGroup},
     {"EnablePlugin", L_EnablePlugin},
     {"EnableTimer", L_EnableTimer},
     {"EnableTimerGroup", L_EnableTimerGroup},
     {"EnableTrigger", L_EnableTrigger},
     {"EnableTriggerGroup", L_EnableTriggerGroup},
     {"IsAlias", L_IsAlias},
     {"IsTimer", L_IsTimer},
     {"IsTrigger", L_IsTrigger},
     // variables
     {"GetVariable", L_GetVariable},
     {"GetPluginVariable", L_GetPluginVariable},
     {"SetVariable", L_SetVariable},
     // windows
     {"Redraw", L_Redraw},
     {"Repaint", L_Repaint},
     {"WindowCreate", L_WindowCreate},
     {"WindowPosition", L_WindowPosition},
     {"WindowResize", L_WindowResize},
     {"WindowSetZOrder", L_WindowSetZOrder},
     {"WindowShow", L_WindowShow},
     // window hotspots
     {"WindowAddHotspot", L_WindowAddHotspot},
     {"WindowDeleteHotspot", L_WindowDeleteHotspot},
     {"WindowMoveHotspot", L_WindowMoveHotspot},

     {NULL, NULL}};

static int L_world_tostring(lua_State *L)
{
  lua_pushliteral(L, "world");
  return 1;
}

static const struct luaL_Reg worldlib_meta[] = {{"__tostring", L_world_tostring}, {NULL, NULL}};

void registerLuaWorld(lua_State *L)
{
  luaL_newlib(L, worldlib);

  luaL_newmetatable(L, WORLD_REG_KEY);
  luaL_setfuncs(L, worldlib_meta, 0);
  lua_setmetatable(L, -2);

  lua_setglobal(L, WORLD_LIB_KEY);

  lua_pushglobaltable(L);
  if (lua_getmetatable(L, -1) == LUA_TNIL)
    lua_newtable(L);

  lua_getglobal(L, WORLD_LIB_KEY);
  lua_setfield(L, -2, "__index");
  lua_setmetatable(L, -2);

  lua_pop(L, 1);
}
