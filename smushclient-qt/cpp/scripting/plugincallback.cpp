#include "plugincallback.h"
#include "qlua.h"
#include "smushclient_qt/src/bridge.cxxqt.h"
extern "C"
{
#include "lua.h"
}

using std::string_view;

// Abstract

bool PluginCallback::findCallback(lua_State *L) const
{
  switch (lua_getglobal(L, name()))
  {
  case LUA_TFUNCTION:
    return true;
  case LUA_TTABLE:
    break;
  default:
    lua_pop(L, 1);
    return false;
  }

  const char *prop = property();

  if (!prop) [[unlikely]]
  {
    lua_pop(L, 1);
    return false;
  }

  if (lua_getfield(L, -1, prop) != LUA_TFUNCTION) [[unlikely]]
  {
    lua_pop(L, 2);
    return false;
  }

  lua_remove(L, -2);
  return true;
}

void DiscardCallback::collectReturned(lua_State *L)
{
  if (processing && !lua_isnil(L, -1) && !lua_toboolean(L, -1))
    processing = false;
}

void ModifyTextCallback::collectReturned(lua_State *L)
{
  size_t len;
  const char *message = lua_tolstring(L, -1, &len);
  if (!message)
    return;
  text.clear();
  text.append(message, len);
}

int ModifyTextCallback::pushArguments(lua_State *L) const
{
  qlua::pushBytes(L, text);
  return 1;
}

// Concrete

int OnPluginBroadcast::pushArguments(lua_State *L) const
{
  lua_pushinteger(L, message);
  qlua::pushString(L, pluginID);
  qlua::pushString(L, pluginName);
  qlua::pushString(L, text);
  return 4;
}

int OnPluginCommand::pushArguments(lua_State *L) const
{
  qlua::pushBytes(L, text);
  return 1;
}

int OnPluginLineReceived::pushArguments(lua_State *L) const
{
  qlua::pushString(L, line);
  return 1;
}

int OnPluginMXPSetEntity::pushArguments(lua_State *L) const
{
  qlua::pushString(L, value);
  return 1;
}

int OnPluginMXPSetVariable::pushArguments(lua_State *L) const
{
  qlua::pushString(L, variable);
  qlua::pushString(L, contents);
  return 2;
}

int OnPluginSend::pushArguments(lua_State *L) const
{
  qlua::pushBytes(L, text);
  return 1;
}

int OnPluginSent::pushArguments(lua_State *L) const
{
  qlua::pushBytes(L, text);
  return 1;
}

int OnPluginTelnetRequest::pushArguments(lua_State *L) const
{
  lua_pushinteger(L, code);
  qlua::pushString(L, message);
  return 2;
}

int OnPluginTelnetSubnegotiation::pushArguments(lua_State *L) const
{
  lua_pushinteger(L, code);
  qlua::pushBytes(L, data);
  return 2;
}

void CallbackFilter::scan(lua_State *L)
{
  static QByteArray emptyByteArray;
  const static OnPluginBroadcast onBroadcast(0, "", "", "");
  const static OnPluginCommand onCommand(CommandSource::User, emptyByteArray);
  const static OnPluginCommandChanged onCommandChanged;
  const static OnPluginClose onClose;
  const static OnPluginCommandEntered onCommandEntered(CommandSource::User, emptyByteArray);
  const static OnPluginConnect onConnect;
  const static OnPluginDisconnect onDisconnect;
  const static OnPluginGetFocus onGetFocus;
  const static OnPluginIacGa onIacGa;
  const static OnPluginInstall onInstall;
  const static OnPluginLineReceived onLineReceived("");
  const static OnPluginListChanged onListChanged;
  const static OnPluginLoseFocus onLoseFocus;
  const static OnPluginMXPStart onMxpStart;
  const static OnPluginMXPStop onMxpStop;
  const static OnPluginMXPSetEntity onMxpSetEntity("");
  const static OnPluginMXPSetVariable onMxpSetVariable("", "");
  const static OnPluginSaveState onSaveState;
  const static OnPluginSend onSend(emptyByteArray);
  const static OnPluginSent onSent(emptyByteArray);
  const static OnPluginTabComplete onTabComplete(emptyByteArray);
  const static OnPluginTelnetRequest onTelnetRequest(0, "");
  const static OnPluginTelnetSubnegotiation onTelnetSubnegotiation(0, emptyByteArray);
  const static OnPluginWorldSave onWorldSave;
  const static OnPluginWorldOutputResized onWorldOutputResized;
  const int top = lua_gettop(L);
  setIfDefined(L, onBroadcast);
  setIfDefined(L, onCommand);
  setIfDefined(L, onCommandChanged);
  setIfDefined(L, onClose);
  setIfDefined(L, onCommandEntered);
  setIfDefined(L, onConnect);
  setIfDefined(L, onDisconnect);
  setIfDefined(L, onGetFocus);
  lua_settop(L, top);
  setIfDefined(L, onIacGa);
  setIfDefined(L, onInstall);
  setIfDefined(L, onLineReceived);
  setIfDefined(L, onListChanged);
  setIfDefined(L, onLoseFocus);
  setIfDefined(L, onMxpStart);
  setIfDefined(L, onMxpStop);
  lua_settop(L, top);
  setIfDefined(L, onMxpSetEntity);
  setIfDefined(L, onMxpSetVariable);
  setIfDefined(L, onSaveState);
  setIfDefined(L, onSend);
  setIfDefined(L, onSent);
  setIfDefined(L, onTabComplete);
  setIfDefined(L, onTelnetRequest);
  setIfDefined(L, onTelnetSubnegotiation);
  setIfDefined(L, onWorldSave);
  setIfDefined(L, onWorldOutputResized);
  lua_settop(L, top);
}

bool CallbackFilter::setIfDefined(lua_State *L, const PluginCallback &callback)
{
  const bool isDefined = lua_getglobal(L, callback.name()) == LUA_TFUNCTION;
  if (isDefined)
    filter |= callback.id();
  return isDefined;
}
