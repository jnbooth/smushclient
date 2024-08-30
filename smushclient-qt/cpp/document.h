#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QtCore/QVector>
#include <QtGui/QTextCursor>
#include <QtWidgets/QLineEdit>
#include <QtNetwork/QTcpSocket>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QMainWindow>

using QVector_QColor = QVector<QColor>;

struct Link;

class Document
{
public:
  explicit Document(QTcpSocket *socket);

  void appendLine();
  void appendText(const QString &text, int format);
  void appendText(const QString &text, quint16 style, const QColor &foreground, const QColor &background, const Link &link);
  void appendText(const QString &text, quint16 style, const QColor &foreground, const QColor &background);
  void scrollToBottom();
  void displayStatusMessage(const QString &status);

  void setBrowser(QTextBrowser *browser);
  void setInput(const QString &text);
  void setLineEdit(QLineEdit *input);
  void setPalette(const QVector<QColor> &palette);

private:
  QLineEdit *input;
  QMainWindow *window;
  QTcpSocket *socket;
  QTextBrowser *browser;
  QTextCursor cursor;
  QTextCharFormat formats[166];
};

#endif // DOCUMENT_H
