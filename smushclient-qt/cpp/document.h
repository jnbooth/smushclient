#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QtGui/QTextCursor>
#include <QtWidgets/QLineEdit>
#include <QtNetwork/QTcpSocket>
#include <QtWidgets/QTextBrowser>

struct Link;

class Document
{
public:
  explicit Document(QTcpSocket *socket);

  void appendLine();
  void appendText(const QString &text);
  void appendText(const QString &text, quint16 style, const QColor &foreground, const QColor &background, const Link &link);
  void appendText(const QString &text, quint16 style, const QColor &foreground, const QColor &background);
  void displayError(const QString &text);
  void scrollToBottom();
  void setInput(const QString &text);
  void setBrowser(QTextBrowser *browser);
  void setLineEdit(QLineEdit *input);

private:
  QLineEdit *input;
  QTcpSocket *socket;
  QTextBrowser *browser;
  QTextCursor cursor;
};

#endif // DOCUMENT_H
