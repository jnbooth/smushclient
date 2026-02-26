#include "plugincallback.h"
#include "../qlua.h"
extern "C"
{
#include "lua.h"
}

using std::string;
using std::string_view;

// Abstract

DynamicPluginCallback::DynamicPluginCallback(const QString& callback)
{
  const qsizetype n = callback.indexOf(u'.');
  if (n == -1) {
    name = callback.toStdString();
    return;
  }
  name = callback.first(n).toStdString();
  property = callback.sliced(n + 1).toStdString();
}

bool
DynamicPluginCallback::findCallback(lua_State* L) const
{
  const int type = lua_getglobal(L, name.c_str());
  if (property.empty()) {
    if (type == LUA_TFUNCTION) {
      return true;
    }
    lua_pop(L, 1);
    return false;
  }
  if (type != LUA_TTABLE) {
    lua_pop(L, 1);
    return false;
  }
  if (lua_getfield(L, -1, property.c_str()) != LUA_TFUNCTION) {
    lua_pop(L, 2);
    return false;
  }
  lua_remove(L, -2);
  return true;
}

bool
NamedPluginCallback::findCallback(lua_State* L) const
{
  if (lua_getglobal(L, name()) == LUA_TFUNCTION) {
    return true;
  }
  lua_pop(L, 1);
  return false;
}

void
DiscardCallback::collectReturned(lua_State* L)
{
  if (processing && !lua_isnil(L, -1) &&
      !static_cast<bool>(lua_toboolean(L, -1))) {
    processing = false;
  }
}

void
ModifyTextCallback::collectReturned(lua_State* L)
{
  size_t len;
  const char* message = lua_tolstring(L, -1, &len);
  if (message == nullptr) {
    return;
  }
  text.clear();
  text.append(message, static_cast<qsizetype>(len));
}

int
ModifyTextCallback::pushArguments(lua_State* L) const
{
  qlua::push(L, text);
  return 1;
}

// Concrete

int
OnPluginBroadcast::pushArguments(lua_State* L) const
{
  lua_pushinteger(L, message);
  qlua::push(L, pluginID);
  qlua::push(L, pluginName);
  qlua::push(L, text);
  return 4;
}

int
OnPluginCommand::pushArguments(lua_State* L) const
{
  qlua::push(L, text);
  return 1;
}

int
OnPluginLineReceived::pushArguments(lua_State* L) const
{
  qlua::push(L, line);
  return 1;
}

int
OnPluginMXPSetEntity::pushArguments(lua_State* L) const
{
  qlua::push(L, value);
  return 1;
}

int
OnPluginMXPSetVariable::pushArguments(lua_State* L) const
{
  qlua::push(L, variable);
  qlua::push(L, contents);
  return 2;
}

int
OnPluginSend::pushArguments(lua_State* L) const
{
  qlua::push(L, text);
  return 1;
}

int
OnPluginSent::pushArguments(lua_State* L) const
{
  qlua::push(L, text);
  return 1;
}

int
OnPluginTelnetRequest::pushArguments(lua_State* L) const
{
  lua_pushinteger(L, code);
  qlua::push(L, message);
  return 2;
}

int
OnPluginTelnetSubnegotiation::pushArguments(lua_State* L) const
{
  lua_pushinteger(L, code);
  qlua::push(L, data);
  return 2;
}

int
TimerCallback::pushArguments(lua_State* L) const
{
  lua_pushlstring(L, label.data(), label.size());
  return 1;
}
