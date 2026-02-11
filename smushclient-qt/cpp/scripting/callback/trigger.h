#pragma once
#include "../scriptthread.h"
#include <QtCore/QPointer>

class PluginCallback;
class Plugin;
struct lua_State;

class CallbackTrigger
{
public:
  CallbackTrigger(const Plugin& plugin,
                  const PluginCallback& callback,
                  QObject* parent);
  CallbackTrigger(CallbackTrigger&& other) noexcept;
  ~CallbackTrigger() = default;

  CallbackTrigger(const CallbackTrigger&) = delete;
  CallbackTrigger& operator=(const CallbackTrigger&) = delete;
  CallbackTrigger& operator=(CallbackTrigger&&) = delete;

  constexpr bool belongsToPlugin(const Plugin& otherPlugin) const noexcept
  {
    return &otherPlugin == plugin;
  }
  bool trigger();
  constexpr bool valid() const noexcept { return isValid; }

private:
  QPointer<QObject> parent;
  const Plugin* plugin;
  ScriptThread thread;
  int top;
  bool isValid;
  int nargs;
};
