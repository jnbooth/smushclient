#pragma once
#include <string>
#include <unordered_map>
#include <QtCore/QPointer>
#include <QtCore/QString>

class ScriptApi;
struct lua_State;

enum struct RunScriptResult
{
  Ok,
  Disabled,
  CompileError,
  RuntimeError,
};

class ScriptState
{
public:
  ScriptState(ScriptApi *api);
  ScriptState(ScriptState &&other);
  ~ScriptState();

  ScriptState(const ScriptState &) = delete;
  ScriptState &operator=(const ScriptState &) = delete;

  void disable();
  RunScriptResult runScript(const QString &script) const;
  QString getError() const;
  void setID(std::string_view pluginID) const;
  std::unordered_map<std::string, std::string> *variables() const;

private:
  lua_State *L;
  bool disabled;
};
