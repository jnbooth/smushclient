#pragma once
#include <QtCore/QString>
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
  ScriptState(ScriptApi *api);
  ScriptState(ScriptState &&other);
  ~ScriptState();

  ScriptState(const ScriptState &) = delete;
  ScriptState &operator=(const ScriptState &) = delete;

  RunScriptResult runScript(const QString &script);
  QString getError() const;

private:
  lua_State *L;
};
