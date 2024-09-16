#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H
#include <vector>
#include <QtCore/QStringList>

class ScriptApi;
struct Plugin;

class WorldTab;

class ScriptEngine
{
public:
  ScriptEngine(ScriptApi *api);
  ScriptEngine(const ScriptEngine &) = delete;
  ScriptEngine &operator=(const ScriptEngine &) = delete;
  ~ScriptEngine();

  void initializeScripts(const QStringList &scripts);
  bool runScript(size_t plugin, const QString &script);

public:
  ScriptApi *api;

private:
  std::vector<Plugin> plugins;

  bool runScript(Plugin &plugin, const QString &script) const;
};

#endif // SCRIPTENGINE_H
