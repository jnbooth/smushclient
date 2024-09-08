#include "scriptengine.h"
#include <QtCore/QCoreApplication>
#include <QtWidgets/QErrorMessage>

ScriptEngine::ScriptEngine() {}

void ScriptEngine::initializeScripts(const QStringList &scripts)
{
  plugins.clear();
  plugins.reserve(scripts.size());
  for (const QString &script : scripts)
  {
    Plugin &plugin = plugins.emplace_back();
    if (!runScript(plugin, script))
      plugin.disabled = true;
  }
}

// Private methods

bool ScriptEngine::runScript(Plugin &plugin, const QString &script)
{
  if (plugin.disabled)
    return false;

  switch (plugin.state.runScript(script))
  {
  case RunScriptResult::Ok:
    return true;
  case RunScriptResult::CompileError:
    qDebug() << "Compile error: " << plugin.state.getError();
    return false;
  case RunScriptResult::RuntimeError:
    qDebug() << "Compile error: " << plugin.state.getError();
    return false;
  }
}
