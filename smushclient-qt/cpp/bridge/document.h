#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QtCore/QVector>
#include <QtGui/QTextBlock>
#include <QtNetwork/QTcpSocket>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QTreeWidgetItem>
#include "rust/cxx.h"

struct SendRequest;
struct SendScriptRequest;
class ScriptApi;
class WorldTab;
enum class SendTarget : int32_t;
struct Link;
struct OutputSpan;

using QVector_QColor = QVector<QColor>;

class Document : public QObject
{
public:
  Document(WorldTab *parent, ScriptApi *api);

  void appendHtml(const QString &html) const;
  void appendLine();
  void appendText(const QString &text, int format) const;
  void appendText(const QString &text, uint16_t style, const QColor &foreground, const QColor &background, const Link &link) const;
  void appendText(const QString &text, uint16_t style, const QColor &foreground, const QColor &background) const;
  void begin() const;
  void echo(const QString &command) const;
  void end() const;
  void eraseLastLine() const;
  void handleMxpChange(bool enabled) const;
  void handleMxpEntity(::rust::str data) const;
  void handleMxpVariable(::rust::str name, ::rust::str value) const;
  void handleTelnetIacGa() const;
  void handleTelnetRequest(uint8_t code, bool supported) const;
  void handleTelnetSubnegotiation(uint8_t code, const QByteArray &data) const;
  bool permitLine(::rust::str line) const;
  void playSound(const QString &filePath) const;
  void setPalette(const QVector_QColor &palette);
  void send(const SendRequest &request) const;
  void send(const SendScriptRequest &request) const;

private:
  ScriptApi *api;
  int lastLine;
  QTextDocument *doc;
  QTextCharFormat formats[166];
  QScrollBar *scrollBar;

  WorldTab *tab() const;
};

#endif // DOCUMENT_H
