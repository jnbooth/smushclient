#pragma once

#include <QtCore/QObject>
#include <QtGui/QTextCursor>
#include <QtGui/QTextDocument>
#include <QtWidgets/QLineEdit>
#include <QtNetwork/QTcpSocket>
#include "scriptenums.h"

class ScriptApi : public QObject
{
  Q_OBJECT

public:
  ScriptApi(QTextDocument *document, QLineEdit *input, QTcpSocket *socket);

  void ColourTell(const QColor &foreground, const QColor &background, const QString &text);
  void Tell(const QString &text);
  ScriptReturnCode Send(const QByteArrayView &bytes);

  void ensureNewline();
  void insertBlock();
  std::unordered_map<std::string, std::string> *getVariableMap(const std::string &pluginID);
  void setVariableMap(const std::string &pluginID, std::unordered_map<std::string, std::string> *variableMap);
  bool unsetVariableMap(const std::string &pluginID, std::unordered_map<std::string, std::string> *variableMap);

private:
  QTextCursor cursor;
  QTextDocument *document;
  QLineEdit *input;
  QTcpSocket *socket;
  std::unordered_map<std::string, std::unordered_map<std::string, std::string> *> variables;
  bool needsNewline;
};
