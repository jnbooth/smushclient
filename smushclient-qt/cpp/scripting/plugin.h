#pragma once
#include <string>
#include <unordered_map>
#include <QtCore/QPointer>
#include <QtCore/QString>

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
  QString getError() const;
  bool hasFunction(std::string_view name) const;
  inline const QString &id() const noexcept { return metadata.id; }
  inline bool disabled() const noexcept { return isDisabled; };
  inline const QString &name() const noexcept { return metadata.name; }
  bool runCallback(std::string_view name, int arg1, std::string_view arg2) const;
  bool runCallbackThreaded(std::string_view name, int arg1, std::string_view arg2, std::string_view arg3, std::string_view arg4) const;
  bool runScript(const QString &script) const;
  inline lua_State *state() const noexcept { return L; }

private:
  lua_State *L;
  bool isDisabled;
  PluginMetadata metadata;

  bool findCallback(std::string_view name) const;
};
