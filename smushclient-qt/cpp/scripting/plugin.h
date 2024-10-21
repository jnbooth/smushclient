#pragma once
#include <string>
#include <unordered_map>
#include <QtCore/QPointer>
#include <QtCore/QString>
#include "plugincallback.h"

class ScriptApi;
struct lua_State;

struct PluginMetadata
{
  const QString id;
  const size_t index;
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
  bool hasFunction(const char *name) const;
  inline bool hasFunction(std::string_view name) const { return hasFunction(name.data()); }
  inline const QString &id() const noexcept { return metadata.id; }
  inline bool disabled() const noexcept { return isDisabled; }
  inline const QString &name() const noexcept { return metadata.name; }
  bool runCallback(PluginCallback &callback) const;
  bool runCallbackThreaded(PluginCallback &callback) const;
  bool runScript(const QString &script) const;
  inline lua_State *state() const noexcept { return L; }

private:
  lua_State *L;
  bool moved;
  bool isDisabled;
  PluginMetadata metadata;

  bool findCallback(const PluginCallback &callback) const;
};
