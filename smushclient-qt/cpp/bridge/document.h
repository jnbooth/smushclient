#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "rust/cxx.h"
#include <QtCore/QVector>
#include <QtGui/QTextCursor>

struct SendRequest;
struct SendScriptRequest;
class MudScrollBar;
class ScriptApi;
class WorldTab;
enum class AliasOutcome : uint8_t;
enum class SendTarget;
enum class TelnetSource : uint8_t;
enum class TelnetVerb : uint8_t;
enum class TextStyle : uint16_t;
struct Link;
struct OutputSpan;

using AliasOutcomes = QFlags<AliasOutcome>;

class Document : public QObject
{
  Q_OBJECT

public:
  Document(WorldTab* parent, ScriptApi* api);

  void appendHtml(const QString& html) const;
  void appendLine();
  void appendExpiringLink(const QString& text,
                          const QTextCharFormat& format,
                          const rust::Str expires);
  void appendText(const QString& text, const QTextCharFormat& format) const;
  void applyStyles(int start, int end, const QTextCharFormat& format) const;
  void beep() const;
  void begin() const;
  void createMxpStat(const QString& entity,
                     const QString& caption,
                     const QString& max) const;
  void end(bool hadOutput);
  void eraseCurrentLine() const;
  void eraseLastCharacter() const;
  void eraseLastLine() const;
  void expireLinks(rust::Str expires);
  void handleMxpChange(bool enabled) const;
  void handleMxpEntity(rust::Str data) const;
  void handleMxpVariable(rust::Str name, rust::Str value) const;
  void handleServerStatus(const QByteArray& variable, const QByteArray& value);
  void handleTelnetGoAhead() const;
  void handleTelnetNaws() const;
  void handleTelnetNegotiation(TelnetSource source,
                               TelnetVerb verb,
                               uint8_t code);
  void handleTelnetSubnegotiation(uint8_t code, const QByteArray& data) const;
  bool permitLine(rust::Str line) const;
  void resetServerStatus();
  void send(const SendRequest& request) const;
  void send(const SendScriptRequest& request) const;
  void setSuppressEcho(bool suppress) const;
  constexpr const QHash<QString, QString>& serverStatus() const noexcept
  {
    return serverStatuses;
  }
  void updateMxpStat(const QString& entity, const QString& value) const;

signals:
  void newActivity();

private:
  std::vector<QTextCursor>& linksWithExpiration(rust::Str expires);

private:
  ScriptApi* api;
  QTextDocument* doc;
  QTextCharFormat expireLinkFormat{};
  std::unordered_map<std::string, std::vector<QTextCursor>> links{};
  int outputStart = 0;
  MudScrollBar* scrollBar;
  bool serverExpiresLinks = false;
  QHash<QString, QString> serverStatuses{};
};

#endif // DOCUMENT_H
