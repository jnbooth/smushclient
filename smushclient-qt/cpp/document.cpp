#include "document.h"
#include "cxx-qt-gen/ffi.cxxqt.h"
#include <QtGui/QTextDocument>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QApplication>
#include <QtWidgets/QStatusBar>

// Utilities

QMainWindow *getMainWindow(QObject *obj)
{
  if (obj == nullptr)
    return nullptr;

  QObject *parent = obj->parent();
  if (parent == nullptr)
    return nullptr;

  QMainWindow *window = qobject_cast<QMainWindow *>(parent);
  if (window != nullptr)
    return window;

  return getMainWindow(parent);
}

inline bool hasStyle(quint16 flags, TextStyle style) noexcept
{
  return flags & (quint16)style;
}

// Formatting

void applyLink(QTextCharFormat &format, const Link &link) noexcept
{
  QString action = QString(link.action);
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

void applyStyles(QTextCharFormat &format, quint16 style, const QColor &foreground, const QColor &background) noexcept
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

QTextCharFormat foregroundFormat(const QColor &foreground)
{
  QTextCharFormat format;
  format.setForeground(QBrush(foreground));
  return format;
}

static const QTextCharFormat errorFormat = foregroundFormat(QColor::fromRgb(127, 0, 0));
static const QTextCharFormat pluginFormat = foregroundFormat(QColor::fromRgb(1, 164, 151));

// Document

Document::Document(QTcpSocket *socket) : socket(socket), window(getMainWindow(socket)) {}

void Document::appendLine()
{
  cursor.insertBlock();
}

void Document::appendText(const QString &text, int foreground)
{
  cursor.insertText(text, formats[foreground]);
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

void Document::displayStatusMessage(const QString &status)
{
  window->statusBar()->showMessage(status);
}

void Document::setBrowser(QTextBrowser *textBrowser)
{
  cursor = QTextCursor(textBrowser->document());
  browser = textBrowser;
}

void Document::setInput(const QString &text)
{
  input->setText(text);
}

void Document::setLineEdit(QLineEdit *lineEdit)
{
  input = lineEdit;
}

void Document::setPalette(const QVector_QColor &palette)
{
  QTextCharFormat *format = &formats[0];
  for (QColor color : palette)
  {
    *format = QTextCharFormat();
    format->setForeground(QBrush(color));
    ++format;
  }
}
