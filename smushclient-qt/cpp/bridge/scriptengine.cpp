#include "scriptengine.h"
#include <QtCore/QCoreApplication>
#include "../scripting/plugin.h"
#include "../scripting/scriptapi.h"

inline QString tr(const char *key)
{
  return QCoreApplication::translate("ScriptEngine", key);
}

ScriptEngine::ScriptEngine(ScriptApi *api)
    : api(api) {}

ScriptEngine::~ScriptEngine() {}

void ScriptEngine::initializeScripts(const QStringList &scripts)
{
  plugins.clear();
  plugins.reserve(scripts.size());
  QString error;
  for (auto it = scripts.cbegin(), end = scripts.cend(); it != end; ++it)
  {
    Plugin &plugin = plugins.emplace_back(api, *it);
    if (!runScript(plugin, *++it))
      plugin.disabled = true;
  }
}

bool ScriptEngine::runScript(size_t plugin, const QString &script)
{
  return runScript(plugins.at(plugin), script);
}

// Private methods

bool ScriptEngine::runScript(Plugin &plugin, const QString &script) const
{
  if (plugin.disabled)
    return false;

  switch (plugin.state.runScript(script))
  {
  case RunScriptResult::Ok:
    return true;
  case RunScriptResult::CompileError:
    api->printError(tr("Compile error: %1").arg(plugin.state.getError()));
    return false;
  case RunScriptResult::RuntimeError:
    api->printError(tr("Runtime error: %1").arg(plugin.state.getError()));
    return false;
  }
}
