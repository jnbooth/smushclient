#pragma once
#include "scriptthread.h"
#include <QtCore/QPointer>

class PluginCallback;
struct lua_State;

class CallbackTrigger {
public:
  CallbackTrigger(lua_State *L, int nargs, QObject *parent);
  CallbackTrigger(CallbackTrigger &&other) noexcept;

  CallbackTrigger(const CallbackTrigger &) = delete;
  CallbackTrigger &operator=(const CallbackTrigger &) = delete;

  bool trigger();

private:
  int nargs;
  QPointer<QObject> parent;
  ScriptThread thread;
  int top;
};
