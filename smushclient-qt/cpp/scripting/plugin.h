#pragma once
#include "callback/key.h"
#include "scriptthread.h"
#include <QtCore/QDateTime>

class PluginCallback;
struct PluginPack;
class ScriptApi;
struct lua_State;

struct PluginMetadata
{
  std::string id;
  size_t index;
  QDateTime installed;
  std::string name;

  PluginMetadata(const PluginPack& pack, size_t index);
};

class Plugin
{
public:
  Plugin(ScriptApi& api, const PluginPack& pack, size_t index);

  bool hasFunction(PluginCallbackKey routine) const;
  bool hasFunction(const QString& routine) const;
  constexpr const std::string& id() const noexcept { return metadata.id; }
  bool install(const PluginPack& pack);
  constexpr const QDateTime& installed() const noexcept
  {
    return metadata.installed;
  }
  constexpr bool isDisabled() const noexcept { return disabled; }
  constexpr const std::string& name() const noexcept { return metadata.name; }
  void reset();
  void reset(ScriptApi& api);
  bool runCallback(PluginCallback& callback) const;
  bool runCallbackThreaded(PluginCallback& callback) const;
  bool runFile(const QString& path) const;
  bool runScript(std::string_view script) const;
  void setEnabled(bool enable = true);
  ScriptThread spawnThread() const;
  constexpr lua_State* state() const noexcept { return L_.get(); }
  void updateMetadata(const PluginPack& pack, size_t index);

private:
  bool disabled = false;
  std::shared_ptr<lua_State> L_ = nullptr;
  PluginMetadata metadata;
};
