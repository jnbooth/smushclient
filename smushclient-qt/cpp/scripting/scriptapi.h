#pragma once

#include <QtCore/QObject>
#include <QtGui/QTextCursor>
#include <QtGui/QTextDocument>
#include <QtWidgets/QLineEdit>
#include <QtNetwork/QTcpSocket>
#include <string>
#include <vector>
#include "scriptenums.h"

class WorldTab;

class ScriptApi : public QObject
{
  Q_OBJECT

public:
  ScriptApi(WorldTab *parent);

  void ColourTell(const QColor &foreground, const QColor &background, const QString &text);
  QVariant GetOption(const std::string &name) const;
  ScriptReturnCode SetOption(const std::string &name, const QVariant &variant);
  ScriptReturnCode Send(const QByteArrayView &bytes);
  void Tell(const QString &text);

  void applyWorld(const World &world);
  void echo(const QString &text);
  void finishNote();
  std::unordered_map<std::string, std::string> *getVariableMap(const std::string &pluginID);
  void printError(const QString &message);
  void setVariableMap(const std::string &pluginID, std::unordered_map<std::string, std::string> *variableMap);
  bool unsetVariableMap(const std::string &pluginID, std::unordered_map<std::string, std::string> *variableMap);

private:
  QTextCursor cursor;
  QTextCharFormat echoFormat;
  QTextCharFormat errorFormat;
  int lastTellPosition;
  std::unordered_map<std::string, std::unordered_map<std::string, std::string> *> variables;

  inline WorldTab *tab() const { return (WorldTab *)parent(); }
};
