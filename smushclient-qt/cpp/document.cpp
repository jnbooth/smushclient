#include "document.h"

Document::Document(QTextBrowser *browser)
    : browser(browser), cursor(browser->document())
{
}

void Document::appendText(const QString &text)
{
  cursor.insertText(text);
}
