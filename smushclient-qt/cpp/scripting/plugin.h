#pragma once
#include <string>
#include <QtCore/QPointer>
#include <QtCore/QString>
#include "plugincallback.h"

class ScriptApi;
struct lua_State;

struct PluginMetadata
{
  const std::string id;
  const size_t index;
  const std::string name;
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
  bool hasFunction(const char *name) const;
  inline bool hasFunction(std::string_view name) const { return hasFunction(name.data()); }
  inline const std::string &id() const noexcept { return metadata.id; }
  inline bool disabled() const noexcept { return isDisabled; }
  inline const std::string &name() const noexcept { return metadata.name; }
  void reset(ScriptApi *api);
  bool runCallback(PluginCallback &callback) const;
  bool runCallbackThreaded(PluginCallback &callback) const;
  bool runFile(const QString &path) const;
  bool runScript(const QString &script) const;
  bool runScript(std::string_view script) const;
  inline lua_State *state() const noexcept { return L; }

private:
  lua_State *L;
  bool isDisabled;
  PluginMetadata metadata;
  bool moved;

  bool findCallback(const PluginCallback &callback) const;
};
