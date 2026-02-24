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
  CallbackTrigger(CallbackTrigger&& other) noexcept = default;
  ~CallbackTrigger() = default;

  CallbackTrigger(const CallbackTrigger&) = delete;
  CallbackTrigger& operator=(const CallbackTrigger&) = delete;
  CallbackTrigger& operator=(CallbackTrigger&&) = delete;

  bool belongsToPlugin(const Plugin& other) const noexcept
  {
    return &other == plugin;
  }
  bool trigger();
  bool valid() const noexcept { return isValid; }

private:
  QPointer<QObject> parent;
  const Plugin* plugin;
  ScriptThread thread;
  int top;
  bool isValid;
  int nargs;
};
