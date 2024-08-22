#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QtGui/QTextCursor>
#include <QtWidgets/QTextBrowser>

class Document
{
public:
  explicit Document();
  explicit Document(QTextBrowser *browser);

  void setBrowser(QTextBrowser *browser);
  void appendLine();
  void appendText(const QString &text, quint16 style, const QColor &foreground, const QColor &background);
  void scrollToBottom();

private:
  QTextBrowser *browser;
  QTextCursor cursor;
};

#endif // DOCUMENT_H
