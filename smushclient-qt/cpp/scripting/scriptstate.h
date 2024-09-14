#pragma once
#include <QtCore/QString>
#include <QtCore/QPointer>
#include "scriptapi.h"

struct lua_State;

enum struct RunScriptResult
{
  Ok,
  CompileError,
  RuntimeError
};

class ScriptState
{
public:
  ScriptState(ScriptApi *api, const QString &pluginID);
  ScriptState(ScriptState &&other);
  ~ScriptState();

  ScriptState(const ScriptState &) = delete;
  ScriptState &operator=(const ScriptState &) = delete;

  RunScriptResult runScript(const QString &script);
  QString getError() const;

private:
  QPointer<ScriptApi> api;
  lua_State *L;
  QString pluginID;
};
