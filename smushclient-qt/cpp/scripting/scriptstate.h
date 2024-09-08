#ifndef SCRIPTSTATE_H
#define SCRIPTSTATE_H

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
  explicit ScriptState();
  ScriptState(ScriptState &&other);
  ~ScriptState();

  ScriptState(const ScriptState &) = delete;
  ScriptState &operator=(const ScriptState &) = delete;

  RunScriptResult runScript(const QString &script);
  QString getError() const;

private:
  lua_State *L;
  QHash<QString, int> chunks;
};

#endif // SCRIPTSTATE_H
