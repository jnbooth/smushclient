#pragma once
#include <QtCore/QStringList>
#include <QtGui/QTextCharFormat>
#include <QtGui/QTextCursor>
#include <QtGui/QTextDocument>
#include <QtWidgets/QLineEdit>
#include <vector>
#include "scriptapi.h"
#include "plugin.h"

class ScriptEngine
{
public:
  ScriptEngine(QTextDocument *document, QLineEdit *input, QTcpSocket *socket);

  ScriptEngine(const ScriptEngine &) = delete;
  ScriptEngine &operator=(const ScriptEngine &) = delete;

  inline void ensureNewline() { api.ensureNewline(); }
  void setErrorFormat(const QTextCharFormat &format);
  void initializeScripts(const QStringList &scripts);
  bool runScript(size_t plugin, const QString &script);

private:
  ScriptApi api;
  QTextCursor cursor;
  std::vector<Plugin> plugins;

  bool runScript(Plugin &plugin, const QString &script);
};
