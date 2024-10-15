#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QtCore/QVector>
#include <QtNetwork/QTcpSocket>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QTreeWidgetItem>
#include "rust/cxx.h"

struct Link;
class ScriptApi;
enum class SendTarget : int32_t;
class WorldTab;

using QVector_QColor = QVector<QColor>;

class Document : public QObject
{
public:
  Document(WorldTab *parent, ScriptApi *api);

  void appendHtml(const QString &html);
  void appendLine();
  void appendText(const QString &text, int format);
  void appendText(const QString &text, uint16_t style, const QColor &foreground, const QColor &background, const Link &link);
  void appendText(const QString &text, uint16_t style, const QColor &foreground, const QColor &background);
  void begin();
  void end() const;
  void handleMxpChange(bool enabled) const;
  void handleMxpEntity(::rust::str data) const;
  void handleMxpVariable(::rust::str name, ::rust::str value) const;
  void handleTelnetIacGa() const;
  void handleTelnetRequest(uint8_t code, bool supported) const;
  void handleTelnetSubnegotiation(uint8_t code, const QByteArray &data) const;
  bool permitLine(::rust::str line) const;
  void playSound(const QString &filePath) const;
  void setPalette(const QVector_QColor &palette);
  void send(size_t plugin, SendTarget target, const QString &text) const;
  void send(
      size_t plugin,
      const QString &callback,
      const QString &alias,
      const QString &line,
      const QStringList &wildcards) const;
  void startTimer(
      size_t plugin,
      SendTarget target,
      const QString &text,
      uint64_t ms,
      bool activeClosed) const;

private:
  ScriptApi *api;
  QTextCursor cursor;
  QTextCharFormat formats[166];
  QScrollBar *scrollBar;

  WorldTab *tab() const;
};

#endif // DOCUMENT_H
