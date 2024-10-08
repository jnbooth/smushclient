#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QtCore/QVector>
#include <QtNetwork/QTcpSocket>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QTreeWidgetItem>

struct Link;
class ScriptApi;
class WorldTab;

using QVector_QColor = QVector<QColor>;

class Document : public QObject
{
public:
  Document(WorldTab *parent, ScriptApi *api);

  void appendLine();
  void appendText(const QString &text, int format);
  void appendText(const QString &text, uint16_t style, const QColor &foreground, const QColor &background, const Link &link);
  void appendText(const QString &text, uint16_t style, const QColor &foreground, const QColor &background);
  void handleMxpChange(bool enabled) const;
  void handleMxpEntity(const char *data, size_t size) const;
  void handleMxpVariable(const char *name, size_t nameSize, const char *value, size_t valueSize) const;
  void handleTelnetIacGa() const;
  void handleTelnetRequest(uint8_t code, bool sent) const;
  void handleTelnetSubnegotiation(uint8_t code, const QByteArray &data) const;
  bool permitLine(const char *data, size_t size) const;
  void scrollToBottom() const;
  void setPalette(const QVector_QColor &palette);
  void send(int32_t target, size_t plugin, const QString &text) const;
  void send(
      size_t plugin,
      const QString &callback,
      const QString &alias,
      const QString &line,
      const QStringList &wildcards) const;

private:
  ScriptApi *api;
  QTextCursor cursor;
  QTextCharFormat formats[166];
  QScrollBar *scrollBar;

  WorldTab *tab() const;
};

#endif // DOCUMENT_H
