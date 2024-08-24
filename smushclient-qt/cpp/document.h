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

  void appendLine();
  void appendText(const QString &text, quint16 style, const QColor &foreground, const QColor &background, const Link &link);
  void appendText(const QString &text, quint16 style, const QColor &foreground, const QColor &background);
  void scrollToBottom();
  void setBrowser(QTextBrowser *browser);

private:
  QTextBrowser *browser;
  QTextCursor cursor;
};

#endif // DOCUMENT_H
