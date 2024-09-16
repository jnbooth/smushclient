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
using stringmap = std::unordered_map<string, string>;

// utils

inline int returnCode(lua_State *L, ScriptReturnCode code)
{
  lua_pushnumber(L, (lua_Number)code);
  return 1;
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

void setPluginID(lua_State *L, const std::string &pluginID)
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
    lua_pushinteger(L, ScriptApi::RGBColourToCode(option.value<QColor>()));
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
  lua_pushinteger(L, ScriptApi::RGBColourToCode(qlua::getQColor(L, 0)));
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

static int L_Note(lua_State *L)
{
  ScriptApi &api = getApi(L);
  insertTexts(L, api);
  api.finishNote();
  return 0;
}

static int L_RGBColourToName(lua_State *L)
{
  qlua::pushQColor(L, ScriptApi::RGBCodeToColour(qlua::getInt(L, 0)));
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

inline void pushVariable(lua_State *L, const stringmap &vars, const string &name)
{
  if (auto search = vars.find(name); search != vars.end())
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
  const stringmap *vars = getApi(L).getVariableMap(qlua::getString(L, 1));
  if (vars == nullptr)
    lua_pushnil(L);
  else
    pushVariable(L, *vars, qlua::getString(L, 2));
  return 1;
}

static int L_SetVariable(lua_State *L)
{
  (*getVariableMap(L))[qlua::getString(L, 1)] = qlua::getString(L, 2);
  return returnCode(L, ScriptReturnCode::OK);
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
     {"Note", L_Note},
     {"RGBColourToName", L_RGBColourToName},
     {"Send", L_Send},
     {"SendNoEcho", L_SendNoEcho},
     {"SetClipboard", L_SetClipboard},
     {"Tell", L_Tell},
     // variables
     {"GetVariable", L_GetVariable},
     {"GetPluginVariable", L_GetPluginVariable},
     {"SetVariable", L_SetVariable},

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
