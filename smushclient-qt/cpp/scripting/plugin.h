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

class Plugin
{
public:
  Plugin(ScriptApi *api);
  Plugin(Plugin &&other);
  ~Plugin();

  Plugin(const Plugin &) = delete;
  Plugin &operator=(const Plugin &) = delete;

  void disable();
  inline bool isDisabled() const { return disabled; };
  RunScriptResult runScript(const QString &script) const;
  QString getError() const;
  void setID(std::string_view pluginID) const;
  inline lua_State *state() const { return L; }

private:
  lua_State *L;
  bool disabled;
};
