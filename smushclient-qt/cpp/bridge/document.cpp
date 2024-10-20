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
extern "C"
{
#include "lua.h"
}

using std::string;
using std::string_view;
using std::chrono::milliseconds;

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

inline string_view strView(::rust::str str) noexcept
{
  return string_view(str.data(), str.length());
}

// Public methods

Document::Document(WorldTab *parent, ScriptApi *api)
    : QObject(parent),
      api(api),
      scrollBar(parent->ui->output->verticalScrollBar()) {}

void Document::appendHtml(const QString &html)
{
  api->appendHtml(html);
}

void Document::appendLine()
{
  api->startLine();
}

void Document::appendText(const QString &text, int foreground)
{
  api->appendText(text, formats[foreground]);
}

void Document::appendText(const QString &text, uint16_t style, const QColor &foreground, const QColor &background)
{
  QTextCharFormat format;
  applyStyles(format, style, foreground, background);
  api->appendText(text, format);
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
  api->appendText(text, format);
}

void Document::begin()
{
  api->updateTimestamp();
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

void Document::handleMxpEntity(::rust::str data) const
{
  OnPluginMXPSetEntity onMxpSetEntity(strView(data));
  api->sendCallback(onMxpSetEntity);
}

void Document::handleMxpVariable(
    ::rust::str name, ::rust::str value) const
{
  OnPluginMXPSetVariable onMxpSetVariable(strView(name), strView(value));
  api->sendCallback(onMxpSetVariable);
}

void Document::handleTelnetIacGa() const
{
  OnPluginIacGa onIacGa;
  api->sendCallback(onIacGa);
}

void Document::handleTelnetRequest(uint8_t code, bool supported) const
{
  OnPluginTelnetRequest onTelnetRequest(code, "SENT_DO");
  api->sendCallback(onTelnetRequest);
  if (supported)
  {
    OnPluginTelnetRequest onTelnetRequest(code, "WILL");
    api->sendCallback(onTelnetRequest);
  }
}

void Document::handleTelnetSubnegotiation(uint8_t code, const QByteArray &data) const
{
  OnPluginTelnetSubnegotiation onTelnetSubnegotiation(code, data);
  api->sendCallback(onTelnetSubnegotiation);
}

bool Document::permitLine(::rust::str line) const
{
  OnPluginLineReceived onLineReceived(strView(line));
  api->sendCallback(onLineReceived);
  return !onLineReceived.discarded();
}

void Document::playSound(const QString &filePath) const
{
  api->PlaySound(0, filePath);
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
      const QString &senderName,
      const QString &line,
      const QStringList &wildcards)
      : PluginCallback(),
        callback(callback.data()),
        senderName(senderName),
        line(line),
        wildcards(wildcards) {}

  inline constexpr const char *name() const noexcept override { return callback; }
  inline constexpr ActionSource source() const noexcept override { return ActionSource::Unknown; }

  int pushArguments(lua_State *L) const override
  {
    qlua::pushQString(L, senderName);
    qlua::pushQString(L, line);
    qlua::pushQStrings(L, wildcards);
    return 3;
  }

private:
  const char *callback;
  const QString &senderName;
  const QString &line;
  const QStringList &wildcards;
};

class TriggerCallback : public AliasCallback
{
public:
  TriggerCallback(
      const string &callback,
      const QString &senderName,
      const QString &line,
      const QStringList &wildcards,
      const rust::Slice<const OutputSpan> &spans)
      : AliasCallback(callback, senderName, line, wildcards),
        spans(spans) {}

  inline constexpr ActionSource source() const noexcept override { return ActionSource::TriggerFired; }

  int pushArguments(lua_State *L) const override
  {
    const int n = AliasCallback::pushArguments(L);
    const size_t len = spans.length();
    lua_createtable(L, len, 0);
    int i = 0;

    for (const OutputSpan &output : spans)
    {
      const TextSpan *span = output.text_span();
      if (!span)
        continue;
      lua_createtable(L, 0, 5);
      lua_pushinteger(L, span->foreground());
      lua_setfield(L, -2, "textcolour");
      lua_pushinteger(L, span->background());
      lua_setfield(L, -2, "backcolour");
      const rust::Str text = span->text();
      size_t len = text.length();
      lua_pushlstring(L, text.data(), len);
      lua_setfield(L, -2, "text");
      lua_pushinteger(L, len);
      lua_setfield(L, -2, "length");
      lua_pushinteger(L, span->style());
      lua_setfield(L, -2, "style");
      lua_rawseti(L, -2, ++i);
    }

    return n + 1;
  }

private:
  const rust::Slice<const OutputSpan> &spans;
};

void Document::send(
    size_t plugin,
    const QString &callback,
    const QString &sender,
    const QString &line,
    const QStringList &wildcards,
    rust::Slice<const OutputSpan> spans) const
{
  const string callbackName = callback.toStdString();
  if (spans.empty())
  {
    AliasCallback aliasCallback(callbackName, sender, line, wildcards);
    api->sendCallback(aliasCallback, plugin);
  }
  else
  {
    TriggerCallback triggerCallback(callbackName, sender, line, wildcards, spans);
    api->sendCallback(triggerCallback, plugin);
  }
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
