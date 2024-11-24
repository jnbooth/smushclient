#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QtCore/QVector>
#include <QtGui/QTextBlock>
#include <QtGui/QTextCursor>
#include <QtNetwork/QTcpSocket>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QTreeWidgetItem>
#include "rust/cxx.h"

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
using TextStyles = QFlags<TextStyle>;
using QVector_QColor = QVector<QColor>;

class Document : public QObject
{
  Q_OBJECT

public:
  Document(WorldTab *parent, ScriptApi *api);

  void appendHtml(const QString &html) const;
  void appendLine();
  void appendText(const QString &text, int format) const;
  void appendText(const QString &text, TextStyles style, const QColor &foreground, const QColor &background, const Link &link);
  void appendText(const QString &text, TextStyles style, const QColor &foreground, const QColor &background) const;
  void applyStyles(int start, int end, TextStyles style, const QColor &foreground, const QColor &background) const;
  void beep() const;
  void begin() const;
  void end(bool hadOutput);
  void eraseCurrentLine() const;
  void eraseLastCharacter() const;
  void eraseLastLine() const;
  void expireLinks(rust::str expires);
  void handleMxpChange(bool enabled) const;
  void handleMxpEntity(rust::str data) const;
  void handleMxpVariable(rust::str name, rust::str value) const;
  void handleServerStatus(const QByteArray &variable, const QByteArray &value);
  void handleTelnetIacGa() const;
  void handleTelnetNaws() const;
  void handleTelnetNegotiation(TelnetSource source, TelnetVerb verb, uint8_t code) const;
  void handleTelnetSubnegotiation(uint8_t code, const QByteArray &data) const;
  bool permitLine(rust::str line) const;
  void playSound(const QString &filePath) const;
  void send(const SendRequest &request) const;
  void send(const SendScriptRequest &request) const;
  void setPalette(const QVector_QColor &palette);
  void setSuppressEcho(bool suppress) const;
  constexpr const QHash<QString, QString> &serverStatus() const noexcept
  {
    return serverStatuses;
  }

signals:
  void newActivity();

private:
  ScriptApi *api;
  QTextDocument *doc;
  QTextCharFormat expireLinkFormat;
  std::array<QTextCharFormat, 164> formats;
  std::unordered_map<std::string, std::vector<QTextCursor>> links;
  int outputStart;
  MudScrollBar *scrollBar;
  bool serverExpiresLinks;
  QHash<QString, QString> serverStatuses;

  std::vector<QTextCursor> &linksWithExpiration(rust::str expires);
};

#endif // DOCUMENT_H
