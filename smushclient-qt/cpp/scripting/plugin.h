#pragma once
#include <string>
#include <QtCore/QPointer>
#include <QtCore/QDateTime>
#include <QtCore/QString>
#include "plugincallback.h"

class ScriptApi;
struct PluginPack;
struct lua_State;

struct PluginMetadata
{
  std::string id;
  size_t index;
  QDateTime installed;
  std::string name;

  PluginMetadata(const PluginPack &pack, size_t index);
};

class Plugin
{
public:
  Plugin(ScriptApi *api, const PluginPack &pack, size_t index);
  Plugin(Plugin &&other);
  ~Plugin();

  Plugin(const Plugin &) = delete;
  Plugin &operator=(const Plugin &) = delete;

  void disable();
  constexpr bool disabled() const noexcept { return isDisabled; }
  void enable();
  bool hasFunction(PluginCallbackKey routine) const;
  bool hasFunction(const QString &routine) const;
  constexpr const std::string &id() const noexcept { return metadata.id; }
  bool install(const PluginPack &pack);
  constexpr const QDateTime &installed() const noexcept { return metadata.installed; }
  constexpr const std::string &name() const noexcept { return metadata.name; }
  void reset();
  void reset(ScriptApi *api);
  bool runCallback(PluginCallback &callback) const;
  bool runCallbackThreaded(PluginCallback &callback) const;
  bool runFile(const QString &path) const;
  bool runScript(const QString &script) const;
  bool runScript(std::string_view script) const;
  constexpr lua_State *state() const noexcept { return L; }

public:
  PluginMetadata metadata;

private:
  bool findCallback(const PluginCallback &callback) const;

private:
  lua_State *L = nullptr;
  bool isDisabled = false;
};
