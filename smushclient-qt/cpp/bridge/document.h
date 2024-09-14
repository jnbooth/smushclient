#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QtCore/QVector>
#include <QtGui/QTextCursor>
#include <QtWidgets/QLineEdit>
#include <QtNetwork/QTcpSocket>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QTreeWidgetItem>
#include "scriptengine.h"

struct Link;
class ScriptApi;

using QVector_QColor = QVector<QColor>;

class Document : public QObject
{
  Q_OBJECT

public:
  Document(WorldTab *parent, ScriptApi *api);

  void appendLine();
  void appendText(const QString &text, const QTextCharFormat &format);
  void appendText(const QString &text, int format);
  void appendText(const QString &text, quint16 style, const QColor &foreground, const QColor &background, const Link &link);
  void appendText(const QString &text, quint16 style, const QColor &foreground, const QColor &background);
  void runScript(size_t plugin, const QString &script);
  void scrollToBottom();
  void displayStatusMessage(const QString &status);

  void setInput(const QString &text);
  void setPalette(const QVector_QColor &palette);

public:
  ScriptEngine scriptEngine;

private:
  QTextCursor cursor;
  QTextCharFormat formats[166];
  QScrollBar *scrollBar;

  inline WorldTab *tab() const { return (WorldTab *)parent(); }
};

#endif // DOCUMENT_H
