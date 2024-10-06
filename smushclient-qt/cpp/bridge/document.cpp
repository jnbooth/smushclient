#include "document.h"
#include <string>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QStatusBar>
#include "../spans.h"
#include "../scripting/qlua.h"
#include "../scripting/scriptapi.h"
#include "../scripting/plugincallback.h"
#include "../ui/ui_worldtab.h"
#include "../ui/worldtab.h"
#include "cxx-qt-gen/ffi.cxxqt.h"

using std::string;
using std::string_view;

// Private utils

inline void applyStyles(
    QTextCharFormat &format,
    uint16_t style,
    const QColor &foreground,
    const QColor &background)
{
  setStyles(format, QFlags<TextStyle>::fromInt(style));
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


void Document::appendHtml(const QString &html)
{
  cursor.insertHtml(html);
}

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

void Document::appendText(
    const QString &text,
    uint16_t style,
    const QColor &foreground,
    const QColor &background,
    const Link &link)
{
  QTextCharFormat format;
  applyStyles(format, style, foreground, background);
  applyLink(format, link);
  cursor.insertText(text, format);
}

void Document::begin()
{
  setTimestamp(cursor);
}

void Document::end() const
{
  scrollToEnd(*scrollBar);
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

void Document::send(size_t plugin, SendTarget target, const QString &text) const
{
  api->sendTo(plugin, target, text);
}

class AliasCallback : public PluginCallback
{
public:
  AliasCallback(
      const string &callback,
      const QString &line,
      const QString &alias,
      const QStringList &wildcards)
      : PluginCallback(),
        callback(callback.data()),
        alias(alias),
        line(line),
        wildcards(wildcards) {}

  inline constexpr const char *name() const noexcept override { return callback; }
  inline constexpr ActionSource source() const noexcept override { return ActionSource::Unknown; }

  int pushArguments(lua_State *L) const override
  {
    qlua::pushQString(L, alias);
    qlua::pushQString(L, line);
    qlua::pushQStrings(L, wildcards);
    return 3;
  }

private:
  const char *callback;
  const QString &alias;
  const QString &line;
  const QStringList &wildcards;
};

void Document::send(
    size_t plugin,
    const QString &callback,
    const QString &alias,
    const QString &line,
    const QStringList &wildcards) const
{
  const string callbackName = callback.toStdString();
  AliasCallback aliasCallback(callbackName, alias, line, wildcards);
  api->sendCallback(aliasCallback, plugin);
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
