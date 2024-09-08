#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

#include <QtCore/QStringList>
#include <vector>
#include "scriptstate.h"

typedef struct Plugin
{
  bool disabled;
  ScriptState state;
} Plugin;

class ScriptEngine
{
public:
  ScriptEngine();

  ScriptEngine(const ScriptEngine &) = delete;
  ScriptEngine &operator=(const ScriptEngine &) = delete;

  void initializeScripts(const QStringList &scripts);

private:
  std::vector<Plugin> plugins;

  bool runScript(Plugin &plugin, const QString &script);
};

#endif // SCRIPTENGINE_H
