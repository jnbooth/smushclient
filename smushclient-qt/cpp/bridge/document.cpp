#include "document.h"
#include <string>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QStatusBar>
#include "../link.h"
#include "../scripting/scriptapi.h"
#include "../scripting/plugincallback.h"
#include "../ui/ui_worldtab.h"
#include "../ui/worldtab.h"
#include "cxx-qt-gen/ffi.cxxqt.h"

using std::string;
using std::string_view;

// Private utils

constexpr bool hasStyle(uint16_t flags, TextStyle style) noexcept
{
  return flags & (uint16_t)style;
}

inline void applyLink(QTextCharFormat &format, const Link &link) noexcept
{
  format.setAnchor(true);
  format.setAnchorHref(encodeLink(link.sendto, link.action));
  if (!link.hint.isEmpty())
    format.setToolTip(link.hint);
  if (!link.prompts.isEmpty())
    format.setProperty(QTextCharFormat::UserProperty, link.prompts);
}

inline void applyStyles(QTextCharFormat &format, uint16_t style, const QColor &foreground, const QColor &background) noexcept
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

inline void scrollToEnd(QScrollBar &bar)
{
  bar.setValue(bar.maximum());
}

// Public methods

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

void Document::appendText(const QString &text, uint16_t style, const QColor &foreground, const QColor &background)
{
  QTextCharFormat format;
  applyStyles(format, style, foreground, background);
  cursor.insertText(text, format);
}

void Document::appendText(const QString &text, uint16_t style, const QColor &foreground, const QColor &background, const Link &link)
{
  QTextCharFormat format;
  applyStyles(format, style, foreground, background);
  applyLink(format, link);
  cursor.insertText(text, format);
}

void Document::handleMxpChange(bool enabled) const
{
  if (enabled)
  {
    OnPluginMXPStart onMxpStart;
    api->sendCallback(onMxpStart);
  }
  else
  {
    OnPluginMXPStop onMxpStop;
    api->sendCallback(onMxpStop);
  }
}

void Document::handleMxpEntity(const char *data, size_t size) const
{
  string_view value(data, size);
  OnPluginMXPSetEntity onMxpSetEntity(value);
  api->sendCallback(onMxpSetEntity);
}

void Document::handleMxpVariable(
    const char *variableData,
    size_t variableSize,
    const char *contentsData,
    size_t contentsSize) const
{
  string_view variable(variableData, variableSize);
  string_view contents(contentsData, contentsSize);
  OnPluginMXPSetVariable onMxpSetVariable(variable, contents);
  api->sendCallback(onMxpSetVariable);
}

void Document::handleTelnetIacGa() const
{
  OnPluginIacGa onIacGa;
  api->sendCallback(onIacGa);
}

void Document::handleTelnetRequest(uint8_t code, bool sent) const
{
  OnPluginTelnetRequest onTelnetRequest(code, sent ? "SENT_DO" : "WILL");
  api->sendCallback(onTelnetRequest);
}

void Document::handleTelnetSubnegotiation(uint8_t code, const QByteArray &data) const
{
  OnPluginTelnetSubnegotiation onTelnetSubnegotiation(code, data);
  api->sendCallback(onTelnetSubnegotiation);
}

bool Document::permitLine(const char *data, size_t size) const
{
  const string_view line(data, size);
  OnPluginLineReceived onLineReceived(line);
  api->sendCallback(onLineReceived);
  return !onLineReceived.discarded();
}

void Document::scrollToBottom() const
{
  scrollToEnd(*scrollBar);
}

void Document::send(int32_t target, size_t plugin, const QString &text) const
{
  api->sendTo(plugin, (SendTarget)target, text);
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

// Private methods

inline WorldTab *Document::tab() const { return qobject_cast<WorldTab *>(parent()); }
