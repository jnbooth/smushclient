#include "scriptengine.h"
#include <QtCore/QCoreApplication>
#include <QtWidgets/QErrorMessage>

inline QString tr(const char *key)
{
  return QCoreApplication::translate("ScriptEngine", key);
}

ScriptEngine::ScriptEngine(QTextDocument *document, QLineEdit *input)
    : api(document, input),
      cursor(document) {}

void ScriptEngine::initializeScripts(const QStringList &scripts)
{
  plugins.clear();
  plugins.reserve(scripts.size());
  for (const QString &script : scripts)
  {
    Plugin &plugin = plugins.emplace_back(&api);
    if (!runScript(plugin, script))
      plugin.disabled = true;
  }
}

void ScriptEngine::setErrorFormat(const QTextCharFormat &format)
{
  cursor.setCharFormat(format);
}

bool ScriptEngine::runScript(size_t plugin, const QString &script)
{
  return runScript(plugins.at(plugin), script);
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
    cursor.insertText(tr("Compile error: %1").arg(plugin.state.getError()));
    cursor.insertBlock();
    return false;
  case RunScriptResult::RuntimeError:
    cursor.insertText(tr("Runtime error: %1").arg(plugin.state.getError()));
    cursor.insertBlock();
    return false;
  }
}
