#include "plugincallback.h"
#include "../qlua.h"
extern "C"
{
#include "lua.h"
}

using qlua::push;
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

namespace namedCallbacks {
QByteArray emptyByteArray; // NOLINT
constexpr const OnPluginBroadcast onBroadcast({}, {}, {}, {});
constexpr const OnPluginClose onClose;
constexpr const OnPluginCommand onCommand({}, {});
constexpr const OnPluginCommandChanged onCommandChanged;
constexpr const OnPluginCommandEntered onCommandEntered({}, emptyByteArray);
constexpr const OnPluginConnect onConnect;
constexpr const OnPluginDisable onDisable;
constexpr const OnPluginDisconnect onDisconnect;
constexpr const OnPluginEnable onEnable;
constexpr const OnPluginGetFocus onGetFocus;
constexpr const OnPluginIacGa onIacGa;
constexpr const OnPluginInstall onInstall;
constexpr const OnPluginLineReceived onLineReceived({});
constexpr const OnPluginListChanged onListChanged;
constexpr const OnPluginLoseFocus onLoseFocus;
constexpr const OnPluginMXPSetEntity onMXPSetEntity({});
constexpr const OnPluginMXPSetVariable onMXPSetVariable({}, {});
constexpr const OnPluginMXPStart onMXPStart;
constexpr const OnPluginMXPStop onMXPStop;
constexpr const OnPluginPartialLineReceived onPartialLineReceived({});
constexpr const OnPluginPlaySound onPlaySound({});
constexpr const OnPluginSaveState onSaveState;
constexpr const OnPluginSelectionChanged onSelectionChanged;
constexpr const OnPluginSend onSend({});
constexpr const OnPluginSent onSent({});
constexpr const OnPluginTelnetOption onTelnetOption({});
constexpr const OnPluginTelnetRequest onTelnetRequest({}, {});
constexpr const OnPluginTelnetSubnegotiation onTelnetSubnegotiation({}, {});
constexpr const OnPluginWorldSave onWorldSave;
constexpr const OnPluginWorldOutputResized onWorldOutputResized;
constexpr const std::array<const NamedPluginCallback*, 30> list = {
  &onBroadcast,
  &onClose,
  &onCommand,
  &onCommandChanged,
  &onCommandEntered,
  &onConnect,
  &onDisable,
  &onDisconnect,
  &onEnable,
  &onGetFocus,
  &onIacGa,
  &onInstall,
  &onLineReceived,
  &onListChanged,
  &onLoseFocus,
  &onMXPSetEntity,
  &onMXPSetVariable,
  &onMXPStart,
  &onMXPStop,
  &onPartialLineReceived,
  &onPlaySound,
  &onSaveState,
  &onSelectionChanged,
  &onSend,
  &onSent,
  &onTelnetOption,
  &onTelnetRequest,
  &onTelnetSubnegotiation,
  &onWorldSave,
  &onWorldOutputResized,
};
constexpr const std::span<const NamedPluginCallback* const> span(list);
} // namespace namedCallbacks

std::span<const NamedPluginCallback* const>
NamedPluginCallback::list()
{
  return namedCallbacks::span;
}
