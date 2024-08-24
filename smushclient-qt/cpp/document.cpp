#include "document.h"
#include "cxx-qt-gen/ffi.cxxqt.h"
#include <QtGui/QTextDocument>
#include <QtWidgets/QScrollBar>

// Utilities

inline bool hasStyle(quint16 flags, TextStyle style)
{
  return flags & (quint16)style;
}

// Formatting

void applyLink(QTextCharFormat &format, const Link &link)
{
  QString action = link.action;
  switch (link.sendto)
  {
  case SendTo::Internet:
    action.append('\x17');
    break;
  case SendTo::Input:
    action.prepend('\x18');
    break;
  case SendTo::World:
    break;
  }
  format.setAnchor(true);
  format.setAnchorHref(action);
  if (!link.hint.isEmpty())
    format.setToolTip(link.hint);
  if (!link.prompts.isEmpty())
    format.setProperty(QTextCharFormat::UserProperty, link.prompts);
}

void applyStyles(QTextCharFormat &format, quint16 style, const QColor &foreground, const QColor &background)
{
  if (hasStyle(style, TextStyle::Bold))
    format.setFontWeight(QFont::Weight::Bold);

  if (hasStyle(style, TextStyle::Italic))
    format.setFontItalic(true);

  if (hasStyle(style, TextStyle::Strikeout))
    format.setFontStrikeOut(true);

  if (hasStyle(style, TextStyle::Underline))
    format.setFontUnderline(true);

  format.setForeground(QBrush(foreground));

  int red, green, blue;
  background.getRgb(&red, &green, &blue);
  if (red + green + blue != 0)
    format.setBackground(QBrush(background));
}

// Document

Document::Document() {}

Document::Document(QTextBrowser *browser) : browser(browser), cursor(browser->document()) {}

void Document::appendLine()
{
  cursor.insertBlock();
}

void Document::appendText(const QString &text, quint16 style, const QColor &foreground, const QColor &background)
{
  QTextCharFormat format;
  applyStyles(format, style, foreground, background);
  cursor.insertText(text, format);
}

void Document::appendText(const QString &text, quint16 style, const QColor &foreground, const QColor &background, const Link &link)
{
  QTextCharFormat format;
  applyStyles(format, style, foreground, background);
  applyLink(format, link);
  cursor.insertText(text, format);
}

void Document::scrollToBottom()
{
  QScrollBar *scrollbar = browser->verticalScrollBar();
  scrollbar->setValue(scrollbar->maximum());
}

void Document::setBrowser(QTextBrowser *textBrowser)
{
  cursor = QTextCursor(textBrowser->document());
  browser = textBrowser;
}
