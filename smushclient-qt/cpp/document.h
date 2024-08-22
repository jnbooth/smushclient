#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QtGui/QTextCursor>
#include <QtWidgets/QTextBrowser>

class Document
{
public:
  explicit Document(QTextBrowser *browser);

  void appendText(const QString &text);

private:
  QTextBrowser *browser;
  QTextCursor cursor;
};

#endif // DOCUMENT_H
