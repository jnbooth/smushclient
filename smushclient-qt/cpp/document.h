#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QtGui/QTextCursor>
#include <QtWidgets/QTextBrowser>

struct Link;

class Document
{
public:
  explicit Document();
  explicit Document(QTextBrowser *browser);

  void setBrowser(QTextBrowser *browser);
  void appendLine();
  void appendText(const QString &text, quint16 style, const QColor &foreground, const QColor &background);
  void appendText(const QString &text, quint16 style, const QColor &foreground, const QColor &background, const Link &link);
  void scrollToBottom();

private:
  QTextBrowser *browser;
  QTextCursor cursor;
};

#endif // DOCUMENT_H
