#pragma once
#include <QtCore/QStringList>
#include <QtGui/QTextCharFormat>
#include <QtGui/QTextCursor>
#include <QtGui/QTextDocument>
#include <QtWidgets/QLineEdit>
#include <vector>
#include "scriptapi.h"
#include "scriptstate.h"

typedef struct Plugin
{
  bool disabled;
  ScriptState state;

  Plugin(ScriptApi *api) : disabled(false), state(api) {}
} Plugin;

class ScriptEngine
{
public:
  ScriptEngine(QTextDocument *document, QLineEdit *input);

  ScriptEngine(const ScriptEngine &) = delete;
  ScriptEngine &operator=(const ScriptEngine &) = delete;

  void setErrorFormat(const QTextCharFormat &format);
  void initializeScripts(const QStringList &scripts);
  bool runScript(size_t plugin, const QString &script);

private:
  ScriptApi api;
  QTextCursor cursor;
  std::vector<Plugin> plugins;

  bool runScript(Plugin &plugin, const QString &script);
};
