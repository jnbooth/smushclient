#include "document.h"
#include "../scripting/callback/plugincallback.h"
#include "../scripting/qlua.h"
#include "../scripting/scriptapi.h"
#include "../settings.h"
#include "../ui/components/mudbrowser.h"
#include "../ui/components/mudscrollbar.h"
#include "../ui/mudstatusbar/mudstatusbar.h"
#include "../ui/worldtab.h"
#include "smushclient_qt/src/ffi/document.cxxqt.h"
#include <QtGui/QAbstractTextDocumentLayout>
#include <QtGui/QTextBlock>
#include <QtGui/QTextDocumentFragment>
#include <QtWidgets/QStatusBar>
#include <string>
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

namespace {
inline string_view
strView(rust::Str str) noexcept
{
  return string_view(str.data(), str.length());
}
} // namespace

// Public methods

Document::Document(MudBrowser& output, ScriptApi& api, QObject* parent)
  : QObject(parent)
  , api(api)
  , cursor(output.cursor())
  , scrollBar(*output.verticalScrollBar())
{
  expireLinkFormat.setAnchor(false);
  expireLinkFormat.setAnchorHref(QString());
}

void
Document::appendHtml(const QString& html) const
{
  cursor->appendHtml(html);
}

void
Document::appendLine()
{
  outputStart = cursor->document()->blockCount() - 1;
  cursor->startLine();
}

void
Document::appendText(const QString& text, const QTextCharFormat& format) const
{
  cursor->appendText(text, format);
}

void
Document::appendExpiringLink(const QString& text,
                             const QTextCharFormat& format,
                             const rust::Str expires)
{
  QTextDocument* doc = cursor->document();
  const int position = doc->characterCount();
  cursor->appendText(text, format);
  QTextCursor linkCursor(doc);
  linkCursor.setPosition(position - 1);
  if (linkCursor.atBlockEnd()) {
    linkCursor.movePosition(QTextCursor::MoveOperation::NextBlock);
  }
  linkCursor.movePosition(QTextCursor::MoveOperation::End,
                          QTextCursor::MoveMode::KeepAnchor);
  linkCursor.setKeepPositionOnInsert(true);
  linksWithExpiration(expires).push_back(linkCursor);
}

void
Document::applyStyles(int start, int end, const QTextCharFormat& format) const
{
  QTextCursor styleCursor(cursor->document()->findBlockByNumber(outputStart));
  styleCursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, start);
  styleCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, end);
  styleCursor.mergeCharFormat(format);
}

void
Document::beep() const
{
  const QString sound = Settings().getBellSound();
  if (!sound.isEmpty()) {
    api.playFileRaw(sound);
  }
}

void
Document::begin() const
{
  cursor->updateTimestamp();
  scrollBar.setAutoScrollEnabled(false);
}

void
Document::clear() const
{
  cursor->clear();
}

void
Document::createMxpStat(const QString& entity,
                        const QString& caption,
                        const QString& max) const
{
  api.statusBarWidgets().createStat(entity, caption, max);
}

void
Document::echo(const QString& text) const
{
  cursor->echo(text);
}

void
Document::end(bool hadOutput)
{
  scrollBar.setAutoScrollEnabled(true);
  if (hadOutput) {
    emit newActivity();
  }
}

void
Document::eraseCharacters(QTextCursor::MoveOperation direction, int n) const
{
  QTextCursor eraseCursor(cursor->document());
  eraseCursor.movePosition(direction, QTextCursor::MoveMode::KeepAnchor, n);
  eraseCursor.removeSelectedText();
}

void
Document::eraseCurrentLine() const
{
  QTextCursor eraseCursor(cursor->document());
  eraseCursor.select(QTextCursor::SelectionType::BlockUnderCursor);
  eraseCursor.removeSelectedText();
}

void
Document::expireLinks(rust::Str expires)
{
  serverExpiresLinks = true;
  if (!expires.empty()) {
    vector<QTextCursor>& expiredLinks = linksWithExpiration(expires);
    for (QTextCursor& linkCursor : expiredLinks) {
      linkCursor.mergeCharFormat(expireLinkFormat);
    }
    expiredLinks.clear();
    return;
  }
  for (auto& pair : links) {
    vector<QTextCursor>& expiredLinks = pair.second;
    for (QTextCursor& linkCursor : expiredLinks) {
      linkCursor.mergeCharFormat(expireLinkFormat);
    }
    expiredLinks.clear();
  }
}

void
Document::eraseLastLine() const
{
  QTextCursor eraseCursor(
    cursor->document()->findBlockByLineNumber(outputStart));
  eraseCursor.select(QTextCursor::SelectionType::BlockUnderCursor);
  eraseCursor.removeSelectedText();
}

void
Document::handleMxpChange(bool enabled) const
{
  if (enabled) {
    OnPluginMXPStart onMxpStart;
    api.sendCallback(onMxpStart);
  } else {
    OnPluginMXPStop onMxpStop;
    api.sendCallback(onMxpStop);
  }
}

void
Document::handleMxpEntity(rust::Str data) const
{
  OnPluginMXPSetEntity onMxpSetEntity(strView(data));
  api.sendCallback(onMxpSetEntity);
}

void
Document::handleMxpVariable(rust::Str name, rust::Str value) const
{
  OnPluginMXPSetVariable onMxpSetVariable(strView(name), strView(value));
  api.sendCallback(onMxpSetVariable);
}

void
Document::handleServerStatus(const QByteArray& variableBytes,
                             const QByteArray& valueBytes)
{
  const QString variable = QString::fromUtf8(variableBytes);
  const QString value = QString::fromUtf8(valueBytes);
  serverStatuses.insert(variable, value);
  if (variable == QStringLiteral("PLAYERS")) {
    api.statusBarWidgets().setUsers(value);
  }
}

void
Document::handleTelnetGoAhead() const
{
  OnPluginIacGa onIacGa;
  api.sendCallback(onIacGa);
}

void
Document::handleTelnetNaws() const
{
  api.sendNaws();
}

void
Document::handleTelnetNegotiation(TelnetSource source,
                                  TelnetVerb verb,
                                  uint8_t code)
{
  if (source == TelnetSource::Client) {
    if (verb != TelnetVerb::Do) {
      return;
    }

    OnPluginTelnetRequest onTelnetRequest(code, "SENT_DO");
    api.sendCallback(onTelnetRequest);
    return;
  }

  if (code == telnetNAWS) {
    if (verb == TelnetVerb::Do) {
      api.setNawsEnabled(true);
    } else if (verb == TelnetVerb::Dont) {
      api.setNawsEnabled(false);
    }
  }

  if (code == telnetMSSP &&
      (verb == TelnetVerb::Will || verb == TelnetVerb::Wont)) {
    resetServerStatus();
  }

  if (verb == TelnetVerb::Will) {
    OnPluginTelnetRequest onTelnetRequest(code, "WILL");
    api.sendCallback(onTelnetRequest);
  }
}

void
Document::handleTelnetSubnegotiation(uint8_t code, const QByteArray& data) const
{
  OnPluginTelnetSubnegotiation onTelnetSubnegotiation(code, data);
  api.sendCallback(onTelnetSubnegotiation);
}

void
Document::moveCursor(QTextCursor::MoveOperation op, int count) const
{
  cursor->move(op, count);
}

bool
Document::permitLine(rust::Str line) const
{
  OnPluginLineReceived onLineReceived(strView(line));
  api.sendCallback(onLineReceived);
  return !onLineReceived.discarded();
}

void
Document::resetServerStatus()
{
  serverStatuses.clear();
}

void
Document::send(const SendRequest& request) const
{
  api.handleSendRequest(request);
}

class AliasCallback : public DynamicPluginCallback
{
public:
  AliasCallback(rust::Str callback,
                rust::Str senderName,
                rust::Str line,
                const rust::Vec<rust::Str>& wildcards,
                const rust::Vec<NamedWildcard>& namedWildcards)
    : DynamicPluginCallback(callback)
    , senderName(senderName)
    , line(line)
    , wildcards(wildcards)
    , namedWildcards(namedWildcards)
  {
  }

  constexpr ActionSource source() const noexcept override
  {
    return ActionSource::Unknown;
  }

  int pushArguments(lua_State* L) const override
  {
    qlua::push(L, senderName);
    qlua::push(L, line);
    lua_createtable(L,
                    static_cast<int>(wildcards.size()),
                    static_cast<int>(namedWildcards.size()));
    lua_Integer i = 1;
    for (rust::Str wildcard : wildcards) {
      qlua::push(L, wildcard);
      lua_rawseti(L, -2, i);
      ++i;
    }
    for (const NamedWildcard& wildcard : namedWildcards) {
      qlua::push(L, wildcard.name);
      qlua::push(L, wildcard.value);
      lua_rawset(L, -3);
    }
    return 3;
  }

private:
  rust::Str senderName;
  rust::Str line;
  const rust::Vec<rust::Str>& wildcards;
  const rust::Vec<NamedWildcard>& namedWildcards;
};

class TriggerCallback : public AliasCallback
{
public:
  TriggerCallback(rust::Str callback,
                  rust::Str senderName,
                  rust::Str line,
                  const rust::Vec<rust::Str>& wildcards,
                  const rust::Vec<NamedWildcard>& namedWildcards,
                  rust::Slice<const OutputSpan> spans)
    : AliasCallback(callback, senderName, line, wildcards, namedWildcards)
    , spans(spans)
  {
  }

  constexpr ActionSource source() const noexcept override
  {
    return ActionSource::TriggerFired;
  }

  int pushArguments(lua_State* L) const override
  {
    const int n = AliasCallback::pushArguments(L);
    lua_createtable(L, static_cast<int>(spans.length()), 0);
    lua_Integer i = 0;

    for (const OutputSpan& output : spans) {
      const TextSpan* span = output.text_span();
      if (span == nullptr) {
        continue;
      }
      lua_createtable(L, 0, 5);
      lua_pushinteger(L, span->foreground());
      lua_setfield(L, -2, "textcolour");
      lua_pushinteger(L, span->background());
      lua_setfield(L, -2, "backcolour");
      const rust::Str text = span->text();
      const lua_Integer len = static_cast<lua_Integer>(text.length());
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
  rust::Slice<const OutputSpan> spans;
};

void
Document::send(const SendScriptRequest& request) const
{
  if (request.output.empty()) {
    AliasCallback aliasCallback(request.script,
                                request.label,
                                request.line,
                                request.wildcards,
                                request.namedWildcards);
    api.sendCallback(aliasCallback, request.plugin);
  } else {
    TriggerCallback triggerCallback(request.script,
                                    request.label,
                                    request.line,
                                    request.wildcards,
                                    request.namedWildcards,
                                    request.output);
    api.sendCallback(triggerCallback, request.plugin);
  }
}

void
Document::setDynamicColor(DynamicColor dynamic, const QColor& color) const
{
  switch (dynamic) {
    case DynamicColor::TextForeground:
      api.SetForegroundColour(color);
      return;
    case DynamicColor::TextBackground:
      api.SetBackgroundColour(color);
      return;
    case DynamicColor::Highlight:
      api.SetHighlightColour(color);
      return;
    default:
      return;
  }
}

void
Document::setSuppressEcho(bool suppress) const
{
  cursor->setSuppressingEcho(suppress);
}

void
Document::updateMxpStat(const QString& entity, const QString& value) const
{
  api.statusBarWidgets().updateStat(entity, value);
}

// Private methods

vector<QTextCursor>&
Document::linksWithExpiration(rust::Str expires)
{
  return links[string(expires.data(), expires.length())];
}
