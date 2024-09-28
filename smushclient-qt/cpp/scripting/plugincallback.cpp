#include "plugincallback.h"
#include "qlua.h"

using std::string_view;

// Abstract

DiscardCallback::DiscardCallback() : processing(true) {}

void DiscardCallback::collectReturned(lua_State *L)
{
  processing = processing && lua_toboolean(L, -1);
}

ModifyTextCallback::ModifyTextCallback(QByteArray &text)
    : text(text) {}

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

OnPluginBroadcast::OnPluginBroadcast(int message, string_view pluginID, string_view pluginName, string_view text)
    : message(message),
      pluginID(pluginID),
      pluginName(pluginName),
      text(text) {}

int OnPluginBroadcast::pushArguments(lua_State *L) const
{
  lua_pushinteger(L, message);
  qlua::pushString(L, pluginID);
  qlua::pushString(L, pluginName);
  qlua::pushString(L, text);
  return 4;
}

OnPluginCommand::OnPluginCommand(const QByteArray &text)
    : DiscardCallback(),
      text(text) {}

int OnPluginCommand::pushArguments(lua_State *L) const
{
  qlua::pushBytes(L, text);
  return 1;
}

OnPluginCommandEntered::OnPluginCommandEntered(QByteArray &text)
    : ModifyTextCallback(text) {}

OnPluginLineReceived::OnPluginLineReceived(string_view line)
    : DiscardCallback(),
      line(line) {}

int OnPluginLineReceived::pushArguments(lua_State *L) const
{
  qlua::pushString(L, line);
  return 1;
}

OnPluginSend::OnPluginSend(const QByteArray &text)
    : DiscardCallback(),
      text(text) {}

int OnPluginSend::pushArguments(lua_State *L) const
{
  qlua::pushBytes(L, text);
  return 1;
}

OnPluginSent::OnPluginSent(const QByteArray &text)
    : text(text) {}

int OnPluginSent::pushArguments(lua_State *L) const
{
  qlua::pushBytes(L, text);
  return 1;
}

OnPluginTabComplete::OnPluginTabComplete(QByteArray &text)
    : ModifyTextCallback(text) {}

OnPluginTelnetRequest::OnPluginTelnetRequest(uint8_t code, string_view message)
    : code(code),
      message(message) {}

int OnPluginTelnetRequest::pushArguments(lua_State *L) const
{
  lua_pushinteger(L, code);
  qlua::pushString(L, message);
  return 2;
}

OnPluginTelnetSubnegotiation::OnPluginTelnetSubnegotiation(uint8_t code, const QByteArray &data)
    : code(code),
      data(data) {}

int OnPluginTelnetSubnegotiation::pushArguments(lua_State *L) const
{
  lua_pushinteger(L, code);
  qlua::pushBytes(L, data);
  return 2;
}
