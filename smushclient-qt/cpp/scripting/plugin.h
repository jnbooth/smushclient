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

struct PluginMetadata
{
  const QString id;
  const QString name;
};

class Plugin
{
public:
  Plugin(ScriptApi *api, PluginMetadata &&metadata);
  Plugin(Plugin &&other);
  ~Plugin();

  Plugin(const Plugin &) = delete;
  Plugin &operator=(const Plugin &) = delete;

  void disable();
  void enable();
  QString getError() const;
  inline const QString &id() const noexcept { return metadata.id; }
  inline bool disabled() const noexcept { return isDisabled; };
  inline const QString &name() const noexcept { return metadata.name; }
  RunScriptResult runScript(const QString &script) const;
  inline lua_State *state() const noexcept { return L; }
  std::unordered_map<std::string, std::string> *variables() const;

private:
  lua_State *L;
  bool isDisabled;
  PluginMetadata metadata;
};
