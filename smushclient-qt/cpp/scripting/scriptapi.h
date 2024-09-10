#pragma once

#include <QtCore/QObject>
#include <QtGui/QTextCursor>
#include <QtGui/QTextDocument>
#include <QtWidgets/QLineEdit>

class ScriptApi : public QObject
{
  Q_OBJECT

public:
  ScriptApi(QTextDocument *document, QLineEdit *input);

  void ColourTell(const QColor &foreground, const QColor &background, const QString &text);

private:
  QTextCursor cursor;
  QTextDocument *document;
  QLineEdit *input;
};
