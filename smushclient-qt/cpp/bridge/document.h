#pragma once

#include "rust/cxx.h"
#include "smushclient_qt/abstractdocument.h"
#include <QtCore/QPointer>
#include <QtGui/QTextCursor>

class MudBrowser;
class MudCursor;
class MudScrollBar;
class ScriptApi;
class WorldTab;
enum class SendTarget;
enum class TextStyle : uint16_t;
struct Link;
struct OutputSpan;

class Document : public AbstractDocument
{
  Q_OBJECT

public:
  Document(MudBrowser& output, ScriptApi& api, QObject* parent = nullptr);

  void resetServerStatus();
  const QHash<QString, QString>& serverStatus() const noexcept
  {
    return serverStatuses;
  }

public:
  void appendHtml(const QString& html) const override;
  void appendExpiringLink(const QString& text,
                          const QTextCharFormat& format,
                          rust::Str expires) override;
  void appendLine() override;
  void appendText(const QString& text,
                  const QTextCharFormat& format) const override;
  void applyStyles(int start,
                   int end,
                   const QTextCharFormat& format) const override;
  void beep() const override;
  void begin() const override;
  void clear() const override;
  void createMxpStat(const QString& entity,
                     const QString& caption,
                     const QString& max) const override;
  void echo(const QString& text) const override;
  void end(bool hadOutput) override;
  void eraseCharacters(QTextCursor::MoveOperation direction,
                       int n) const override;
  void eraseCurrentLine() const override;
  void eraseLastLine() const override;
  void expireLinks(rust::Str expires) override;
  void handleMxpChange(bool enabled) const override;
  void handleMxpEntity(rust::Str data) const override;
  void handleMxpVariable(rust::Str name, rust::Str value) const override;
  void handleServerStatus(rust::Slice<const uint8_t> variable,
                          rust::Slice<const uint8_t> value) override;
  void handleTelnetGoAhead() const override;
  void handleTelnetNaws() const override;
  void handleTelnetNegotiation(TelnetSource source,
                               TelnetVerb verb,
                               uint8_t code) override;
  void handleTelnetSubnegotiation(uint8_t code, rust::Slice<const uint8_t> data)
    const override;
  void moveCursor(QTextCursor::MoveOperation op, int count) const override;
  bool permitLine(rust::Str line) const override;
  bool permitSound(rust::Str file) const override;
  void send(const SendRequest& request) const override;
  void send(const SendScriptRequest& request) const override;
  void setDynamicColor(DynamicColor dynamic,
                       const QColor& color) const override;
  void setSuppressEcho(bool suppress) const override;
  void updateMxpStat(const QString& entity,
                     const QString& value) const override;

signals:
  void newActivity();

private:
  std::vector<QTextCursor>& linksWithExpiration(rust::Str expires);

private:
  ScriptApi& api;
  QPointer<MudCursor> cursor;
  QTextCharFormat expireLinkFormat;
  std::unordered_map<std::string, std::vector<QTextCursor>> links;
  int outputStart = 0;
  MudScrollBar& scrollBar;
  bool serverExpiresLinks = false;
  QHash<QString, QString> serverStatuses;
};
