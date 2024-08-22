#include "document.h"
#include "cxx-qt-gen/ffi.cxxqt.h"
#include <QtGui/QBrush>
#include <QtGui/QTextCharFormat>
#include <QtGui/QTextDocument>
#include <QtWidgets/QScrollBar>

bool hasStyle(quint16 flags, TextStyle style)
{
  return flags & (quint16)style;
}

bool isBlack(const QColor &color)
{
  int red, green, blue;
  color.getRgb(&red, &green, &blue);
  return red + green + blue == 0;
}

void setColors(QTextFormat &format, const QColor &foreground, const QColor &background)
{
  format.setForeground(QBrush(foreground));
  if (!isBlack(background))
  {
    format.setBackground(QBrush(background));
  }
}

Document::Document() {}

Document::Document(QTextBrowser *browser) : browser(browser), cursor(browser->document())
{
}

void Document::setBrowser(QTextBrowser *browser)
{
  cursor = QTextCursor(browser->document());
  this->browser = browser;
}

void Document::scrollToBottom()
{
  QScrollBar *scrollbar = browser->verticalScrollBar();
  scrollbar->setValue(scrollbar->maximum());
}

void Document::appendLine()
{
  cursor.insertBlock();
}

void Document::appendText(const QString &text, quint16 style, const QColor &foreground, const QColor &background)
{
  QTextCharFormat format;
  if (hasStyle(style, TextStyle::Bold))
  {
    format.setFontWeight(QFont::Weight::Bold);
  }
  if (hasStyle(style, TextStyle::Italic))
  {
    format.setFontItalic(true);
  }
  if (hasStyle(style, TextStyle::Strikeout))
  {
    format.setFontStrikeOut(true);
  }
  if (hasStyle(style, TextStyle::Underline))
  {
    format.setFontUnderline(true);
  }
  if (hasStyle(style, TextStyle::Inverse))
  {
    setColors(format, background, foreground);
  }
  else
  {
    setColors(format, foreground, background);
  }
  cursor.insertText(text, format);
}
