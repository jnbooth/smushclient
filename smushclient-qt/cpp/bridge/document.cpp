#include "document.h"
#include <string>
#include <QtGui/QTextBlock>
#include <QtGui/QTextDocumentFragment>
#include <QtGui/QAbstractTextDocumentLayout>
#include <QtWidgets/QStatusBar>
#include "../spans.h"
#include "../scripting/qlua.h"
#include "../scripting/scriptapi.h"
#include "../scripting/plugincallback.h"
#include "../ui/components/mudscrollbar.h"
#include "../ui/mudstatusbar/mudstatusbar.h"
#include "../ui/ui_worldtab.h"
#include "../ui/worldtab.h"
#include "../../settings.h"
#include "smushclient_qt/src/ffi/document.cxxqt.h"
extern "C"
{
#include "lua.h"
}

using std::string;
using std::string_view;
using std::vector;
using std::chrono::milliseconds;

constexpr uint8_t telnetNAWS = 31;
constexpr uint8_t telnetMSSP = 70;

// Private utils

inline void mergeStyles(
    QTextCharFormat &format,
    TextStyles style,
    const QColor &foreground,
    const QColor &background)
{
  setStyles(format, style);
  if (foreground.isValid())
    format.setForeground(QBrush(foreground));

  if (!background.isValid())
    return;

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

inline string_view strView(rust::str str) noexcept
{
  return string_view(str.data(), str.length());
}

// Public methods

Document::Document(WorldTab *parent, ScriptApi *api)
    : QObject(parent),
      api(api),
      doc(parent->ui->output->document()),
      scrollBar(parent->ui->output->verticalScrollBar())
{
  formats.fill(QTextCharFormat());
  expireLinkFormat.setAnchor(false);
  expireLinkFormat.setAnchorHref(QString());
}

void Document::appendHtml(const QString &html) const
{
  api->appendHtml(html);
}

void Document::appendLine()
{
  outputStart = doc->blockCount() - 1;
  api->startLine();
}

void Document::appendText(const QString &text, int foreground) const
{
  api->appendText(text, formats[foreground]);
}

void Document::appendText(
    const QString &text,
    TextStyles style,
    const QColor &foreground,
    const QColor &background) const
{
  QTextCharFormat format;
  mergeStyles(format, style, foreground, background);
  api->appendText(text, format);
}

void Document::appendText(
    const QString &text,
    TextStyles style,
    const QColor &foreground,
    const QColor &background,
    const Link &link)
{
  QTextCharFormat format;
  mergeStyles(format, style, foreground, background);
  applyLink(format, link);
  const int position = doc->characterCount();
  api->appendText(text, format);

  if (!serverExpiresLinks || link.expires.empty())
    return;

  QTextCursor cursor(doc);
  cursor.setPosition(position - 1);
  if (cursor.atBlockEnd())
    cursor.movePosition(QTextCursor::MoveOperation::NextBlock);
  cursor.movePosition(QTextCursor::MoveOperation::End, QTextCursor::MoveMode::KeepAnchor);
  cursor.setKeepPositionOnInsert(true);
  linksWithExpiration(link.expires).push_back(cursor);
}

void Document::applyStyles(int start, int end, TextStyles style, const QColor &foreground, const QColor &background) const
{
  QTextCharFormat format;
  mergeStyles(format, style, foreground, background);
  QTextCursor cursor(doc->findBlockByNumber(outputStart));
  cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, start);
  cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, end);
  cursor.mergeCharFormat(format);
}

void Document::beep() const
{
  const QString sound = Settings().getBellSound();
  if (!sound.isEmpty())
    api->PlaySound(0, sound);
}

void Document::begin() const
{
  api->updateTimestamp();
  scrollBar->setAutoScrollEnabled(false);
}

void Document::createMxpStat(const QString &entity, const QString &caption, const QString &max) const
{
  api->statusBarWidgets()->createStat(entity, caption, max);
}

void Document::end(bool hadOutput)
{
  scrollBar->setAutoScrollEnabled(true);
  if (hadOutput)
    emit newActivity();
}

void Document::eraseCurrentLine() const
{
  QTextCursor cursor(doc);
  cursor.select(QTextCursor::SelectionType::BlockUnderCursor);
  cursor.removeSelectedText();
}

void Document::eraseLastCharacter() const
{
  QTextCursor cursor(doc);
  cursor.movePosition(QTextCursor::MoveOperation::PreviousCharacter, QTextCursor::MoveMode::KeepAnchor);
  cursor.removeSelectedText();
}

void Document::expireLinks(rust::str expires)
{
  serverExpiresLinks = true;
  if (!expires.empty())
  {
    std::vector<QTextCursor> &expiredLinks = linksWithExpiration(expires);
    for (QTextCursor &cursor : expiredLinks)
      cursor.mergeCharFormat(expireLinkFormat);
    expiredLinks.clear();
    return;
  }
  for (auto &pair : links)
  {
    std::vector<QTextCursor> &expiredLinks = pair.second;
    for (QTextCursor &cursor : expiredLinks)
      cursor.mergeCharFormat(expireLinkFormat);
    expiredLinks.clear();
  }
}

void Document::eraseLastLine() const
{
  QTextCursor cursor(doc->findBlockByLineNumber(outputStart));
  cursor.select(QTextCursor::SelectionType::BlockUnderCursor);
  cursor.removeSelectedText();
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

void Document::handleMxpEntity(rust::str data) const
{
  OnPluginMXPSetEntity onMxpSetEntity(strView(data));
  api->sendCallback(onMxpSetEntity);
}

void Document::handleMxpVariable(rust::str name, rust::str value) const
{
  OnPluginMXPSetVariable onMxpSetVariable(strView(name), strView(value));
  api->sendCallback(onMxpSetVariable);
}

void Document::handleServerStatus(const QByteArray &variableBytes, const QByteArray &valueBytes)
{
  const QString variable = QString::fromUtf8(variableBytes);
  const QString value = QString::fromUtf8(valueBytes);
  serverStatuses.insert(variable, value);
  if (variable == QStringLiteral("PLAYERS"))
    api->statusBarWidgets()->setUsers(value);
}

void Document::handleTelnetGoAhead() const
{
  OnPluginIacGa onIacGa;
  api->sendCallback(onIacGa);
}

void Document::handleTelnetNaws() const
{
  api->sendNaws();
}

void Document::handleTelnetNegotiation(TelnetSource source, TelnetVerb verb, uint8_t code)
{
  if (source == TelnetSource::Client)
  {
    if (verb != TelnetVerb::Do)
      return;

    OnPluginTelnetRequest onTelnetRequest(code, "SENT_DO");
    api->sendCallback(onTelnetRequest);
    return;
  }

  if (code == telnetNAWS)
  {
    if (verb == TelnetVerb::Do)
      api->setNawsEnabled(true);
    else if (verb == TelnetVerb::Dont)
      api->setNawsEnabled(false);
  }

  if (code == telnetMSSP && (verb == TelnetVerb::Will || verb == TelnetVerb::Wont))
    resetServerStatus();

  if (verb == TelnetVerb::Will)
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

bool Document::permitLine(rust::str line) const
{
  OnPluginLineReceived onLineReceived(strView(line));
  api->sendCallback(onLineReceived);
  return !onLineReceived.discarded();
}

void Document::resetServerStatus()
{
  serverStatuses.clear();
}

void Document::send(const SendRequest &request) const
{
  api->sendTo(request.plugin, request.send_to, request.text, request.destination);
}

class AliasCallback : public DynamicPluginCallback
{
public:
  AliasCallback(
      rust::Str callback,
      rust::Str senderName,
      rust::Str line,
      const rust::Vec<rust::Str> &wildcards,
      const rust::Vec<NamedWildcard> &namedWildcards)
      : DynamicPluginCallback(callback),
        senderName(senderName),
        line(line),
        wildcards(wildcards),
        namedWildcards(namedWildcards) {}

  inline constexpr ActionSource source() const noexcept override { return ActionSource::Unknown; }

  int pushArguments(lua_State *L) const override
  {
    qlua::pushRString(L, senderName);
    qlua::pushRString(L, line);
    lua_createtable(L, (int)wildcards.size(), (int)namedWildcards.size());
    int i = 1;
    for (rust::Str wildcard : wildcards)
    {
      qlua::pushRString(L, wildcard);
      lua_rawseti(L, -2, i);
      ++i;
    }
    for (const NamedWildcard &wildcard : namedWildcards)
    {
      qlua::pushRString(L, wildcard.name);
      qlua::pushRString(L, wildcard.value);
      lua_rawset(L, -3);
    }
    return 3;
  }

private:
  rust::Str senderName;
  rust::Str line;
  const rust::Vec<rust::Str> &wildcards;
  const rust::Vec<NamedWildcard> &namedWildcards;
};

class TriggerCallback : public AliasCallback
{
public:
  TriggerCallback(
      rust::Str callback,
      rust::Str senderName,
      rust::Str line,
      const rust::Vec<rust::Str> &wildcards,
      const rust::Vec<NamedWildcard> &namedWildcards,
      rust::Slice<const OutputSpan> spans)
      : AliasCallback(callback, senderName, line, wildcards, namedWildcards),
        spans(spans) {}

  inline constexpr ActionSource source() const noexcept override { return ActionSource::TriggerFired; }

  int pushArguments(lua_State *L) const override
  {
    const int n = AliasCallback::pushArguments(L);
    lua_createtable(L, (int)spans.length(), 0);
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
      const int len = (int)text.length();
      lua_pushlstring(L, text.data(), len);
      lua_setfield(L, -2, "text");
      lua_pushinteger(L, (int)len);
      lua_setfield(L, -2, "length");
      lua_pushinteger(L, span->style());
      lua_setfield(L, -2, "style");
      lua_rawseti(L, -2, ++i);
    }

    return n + 1;
  }

private:
  rust::Slice<const OutputSpan> spans;
};

void Document::send(const SendScriptRequest &request) const
{
  if (request.output.empty())
  {
    AliasCallback aliasCallback(
        request.script,
        request.label,
        request.line,
        request.wildcards,
        request.namedWildcards);
    api->sendCallback(aliasCallback, request.plugin);
  }
  else
  {
    TriggerCallback triggerCallback(
        request.script,
        request.label,
        request.line,
        request.wildcards,
        request.namedWildcards,
        request.output);
    api->sendCallback(triggerCallback, request.plugin);
  }
}

void Document::setPalette(const QVector<QColor> &palette)
{
  QTextCharFormat *format = &formats[0];
  for (QColor color : palette)
  {
    format->setForeground(color);
    ++format;
  }
}

void Document::setSuppressEcho(bool suppress) const
{
  api->setSuppressEcho(suppress);
}

void Document::updateMxpStat(const QString &entity, const QString &value) const
{
  api->statusBarWidgets()->updateStat(entity, value);
}

// Private methods

vector<QTextCursor> &Document::linksWithExpiration(rust::str expires)
{
  return links[string(expires.data(), expires.length())];
}
