#include "filter.h"
#include "plugincallback.h"
#include "smushclient_qt/src/ffi/client.cxxqt.h"
extern "C"
{
#include "lua.h"
}

void
CallbackFilter::scan(lua_State* L)
{
  static QByteArray emptyByteArray;
  const static OnPluginBroadcast onBroadcast(0, "", "", "");
  const static OnPluginCommand onCommand(CommandSource::User, &emptyByteArray);
  const static OnPluginCommandChanged onCommandChanged;
  const static OnPluginClose onClose;
  const static OnPluginCommandEntered onCommandEntered(CommandSource::User,
                                                       &emptyByteArray);
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
  const static OnPluginSend onSend(&emptyByteArray);
  const static OnPluginSent onSent(&emptyByteArray);
  const static OnPluginTabComplete onTabComplete(&emptyByteArray);
  const static OnPluginTelnetRequest onTelnetRequest(0, "");
  const static OnPluginTelnetSubnegotiation onTelnetSubnegotiation(
    0, &emptyByteArray);
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

void
CallbackFilter::setIfDefined(lua_State* L, const NamedPluginCallback& callback)
{
  if (lua_getglobal(L, callback.name()) == LUA_TFUNCTION) {
    filter |= callback.id();
  }
}
