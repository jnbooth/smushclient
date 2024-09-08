#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QtCore/QVector>
#include <QtGui/QTextCursor>
#include <QtWidgets/QLineEdit>
#include <QtNetwork/QTcpSocket>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QTreeWidgetItem>

struct Link;

using QVector_QColor = QVector<QColor>;

class Document : QObject
{
  Q_OBJECT

public:
  Document(QTextBrowser *browser, QLineEdit *input);

  void appendLine();
  void appendText(const QString &text, int format);
  void appendText(const QString &text, quint16 style, const QColor &foreground, const QColor &background, const Link &link);
  void appendText(const QString &text, quint16 style, const QColor &foreground, const QColor &background);
  void scrollToBottom();
  void displayStatusMessage(const QString &status);

  void setInput(const QString &text);
  void setPalette(const QVector_QColor &palette);

private:
  QTextBrowser *browser;
  QTextCursor cursor;
  QTextCharFormat formats[166];
  QLineEdit *input;
};

#endif // DOCUMENT_H
