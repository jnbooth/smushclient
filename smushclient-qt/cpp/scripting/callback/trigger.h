#pragma once
#include "../scriptthread.h"
#include <QtCore/QPointer>

class PluginCallback;
class Plugin;
struct lua_State;

class CallbackTrigger
{
public:
  CallbackTrigger(const Plugin& plugin, const PluginCallback& callback);
  CallbackTrigger(CallbackTrigger&& other) noexcept = default;
  ~CallbackTrigger() = default;

  CallbackTrigger(const CallbackTrigger&) = delete;
  CallbackTrigger& operator=(const CallbackTrigger&) = delete;
  CallbackTrigger& operator=(CallbackTrigger&&) = delete;

  bool belongsToPlugin(const Plugin& plugin) const;
  bool trigger();
  bool valid() const noexcept { return isValid; }

private:
  std::shared_ptr<bool> disabled;
  ScriptThread thread;
  int top;
  int nargs;
  bool isValid;
};
