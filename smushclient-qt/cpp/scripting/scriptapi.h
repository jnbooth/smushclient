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
  ScriptReturnCode Send(const QByteArrayView &bytes);

  void ensureNewline();
  void insertBlock();

private:
  QTextCursor cursor;
  QTextDocument *document;
  QLineEdit *input;
  QTcpSocket *socket;
  bool needsNewline;
};
