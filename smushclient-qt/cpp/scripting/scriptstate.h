#pragma once
#include <QtCore/QHash>
#include <QtCore/QString>

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
  ScriptState();
  ScriptState(ScriptState &&other);
  ~ScriptState();

  ScriptState(const ScriptState &) = delete;
  ScriptState &operator=(const ScriptState &) = delete;

  RunScriptResult runScript(const QString &script);
  QString getError() const;

private:
  lua_State *L;
};
