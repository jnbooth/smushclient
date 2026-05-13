#include "plugincallback.h"
#include "../qlua.h"
extern "C"
{
#include <lua.h>
}

using qlua::push;

// Abstract

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
  if (processing && !lua_isnil(L, -1) && !lua_tobool(L, -1)) {
    processing = false;
  }
}

void
ModifyTextCallback::collectReturned(lua_State* L)
{
  const QByteArrayView message = lua_tobytes(L, -1);
  if (message.isNull()) {
    return;
  }
  text.clear();
  text.append(message);
}

int
ModifyTextCallback::pushArguments(lua_State* L) const
{
  push(L, text);
  return 1;
}

// Concrete

int
OnPluginBroadcast::pushArguments(lua_State* L) const
{
  push(L, message);
  push(L, pluginID);
  push(L, pluginName);
  push(L, text);
  return 4;
}

int
OnPluginCommand::pushArguments(lua_State* L) const
{
  push(L, text);
  return 1;
}

int
OnPluginLineReceived::pushArguments(lua_State* L) const
{
  push(L, line);
  return 1;
}

int
OnPluginMXPSetEntity::pushArguments(lua_State* L) const
{
  push(L, value);
  return 1;
}

int
OnPluginMXPSetVariable::pushArguments(lua_State* L) const
{
  push(L, variable);
  push(L, contents);
  return 2;
}

int
OnPluginPartialLineReceived::pushArguments(lua_State* L) const
{
  push(L, line);
  return 1;
}

int
OnPluginPlaySound::pushArguments(lua_State* L) const
{
  push(L, file);
  return 1;
}

int
OnPluginSend::pushArguments(lua_State* L) const
{
  push(L, text);
  return 1;
}

int
OnPluginSent::pushArguments(lua_State* L) const
{
  push(L, text);
  return 1;
}

int
OnPluginTelnetOption::pushArguments(lua_State* L) const
{
  push(L, message);
  return 1;
}

int
OnPluginTelnetRequest::pushArguments(lua_State* L) const
{
  push(L, code);
  push(L, message);
  return 2;
}

int
OnPluginTelnetSubnegotiation::pushArguments(lua_State* L) const
{
  push(L, code);
  push(L, data);
  return 2;
}

int
TimerCallback::pushArguments(lua_State* L) const
{
  push(L, label);
  return 1;
}

std::span<const NamedPluginCallback::Metadata>
NamedPluginCallback::list()
{
  static QByteArray emptyByteArray;

  static constexpr std::initializer_list<Metadata> list{
    OnPluginBroadcast({}, {}, {}, {}),
    OnPluginClose(),
    OnPluginCommand({}, {}),
    OnPluginCommandChanged(),
    OnPluginCommandEntered(ActionSource::Unknown, emptyByteArray),
    OnPluginConnect(),
    OnPluginDisable(),
    OnPluginDisconnect(),
    OnPluginEnable(),
    OnPluginGetFocus(),
    OnPluginIacGa(),
    OnPluginInstall(),
    OnPluginLineReceived({}),
    OnPluginListChanged(),
    OnPluginLoseFocus(),
    OnPluginMXPSetEntity({}),
    OnPluginMXPSetVariable({}, {}),
    OnPluginMXPStart(),
    OnPluginMXPStop(),
    OnPluginPartialLineReceived({}),
    OnPluginPlaySound({}),
    OnPluginSaveState(),
    OnPluginSelectionChanged(),
    OnPluginSend({}),
    OnPluginSent({}),
    OnPluginTelnetOption({}),
    OnPluginTelnetRequest({}, {}),
    OnPluginTelnetSubnegotiation({}, {}),
    OnPluginWorldSave(),
    OnPluginWorldOutputResized(),
  };
  constexpr std::span<const NamedPluginCallback::Metadata> span(list);
  return span;
}
