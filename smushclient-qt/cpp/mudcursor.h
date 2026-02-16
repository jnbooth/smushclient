#pragma once
#include <QtGui/QTextCursor>

class World;

class MudCursor : public QObject
{
  Q_OBJECT

public:
  explicit MudCursor(QTextDocument* document);

  void appendError(const QString& message);
  void appendHtml(const QString& html);
  void appendTell(const QString& text, const QTextCharFormat& format);
  void appendTell(const QString& text) { appendTell(text, noteFormat); };
  void appendText(const QString& text, const QTextCharFormat& format);
  void appendText(const QString& text) { appendText(text, noteFormat); }
  void applyWorld(const World& world);
  void clear();
  QTextDocument* document() const;
  void echo(const QString& text);
  void finishNote();
  void move(QTextCursor::MoveOperation op, int count);
  void setIndentText(const QString& text);
  void setSuppressingEcho(bool suppress = true);
  constexpr bool suppressingEcho() const noexcept { return suppressEcho; }
  int startLine();
  void setOption(std::string_view name, int64_t value);
  void updateTimestamp();

signals:
  void noteLogged(const QString& note);

private:
  void flushLine();
  void insertBlock();
  void insertText(const QString& text,
                  const QTextCharFormat& format = QTextCharFormat());

private:
  QTextCursor cursor;
  QTextCharFormat echoFormat;
  bool echoOnSameLine = false;
  QTextCharFormat errorFormat;
  bool hasLine = false;
  bool indentNext = false;
  QString indentText;
  int lastLinePosition = -1;
  int lastTellPosition = -1;
  bool logNotes = false;
  QTextCharFormat noteFormat;
  bool suppressEcho = false;
};
