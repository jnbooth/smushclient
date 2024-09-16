#include "document.h"
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QStatusBar>
#include "../scripting/scriptapi.h"
#include "../ui/ui_worldtab.h"
#include "../ui/worldtab.h"
#include "cxx-qt-gen/ffi.cxxqt.h"

// Utilities

QMainWindow *getMainWindow(const QObject *obj)
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

inline void scrollToEnd(QScrollBar &bar)
{
  bar.setValue(bar.maximum());
}

// Formatting

inline void applyLink(QTextCharFormat &format, const Link &link) noexcept
{
  QString action = QString(link.action);
  switch (link.sendto)
  {
  case SendTo::Internet:
    action.append(QChar(17));
    break;
  case SendTo::Input:
    action.prepend(QChar(18));
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

inline void applyStyles(QTextCharFormat &format, quint16 style, const QColor &foreground, const QColor &background) noexcept
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

inline QTextCharFormat foregroundFormat(const QColor &foreground)
{
  QTextCharFormat format;
  format.setForeground(QBrush(foreground));
  return format;
}

// Document

Document::Document(WorldTab *parent, ScriptApi *api)
    : QObject(parent),
      api(api),
      cursor(parent->ui->output->document()),
      scrollBar(parent->ui->output->verticalScrollBar()) {}

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

void Document::runScript(size_t plugin, const QString &script) const
{
  api->runScript(plugin, script);
}

void Document::scrollToBottom() const
{
  scrollToEnd(*scrollBar);
}

void Document::displayStatusMessage(const QString &status) const
{
  QMainWindow *window = getMainWindow(this);
  if (window == nullptr)
    return;

  QStatusBar *statusBar = window->statusBar();
  if (statusBar == nullptr)
    return;

  statusBar->showMessage(status);
}

void Document::setInput(const QString &text) const
{
  tab()->ui->input->setText(text);
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
