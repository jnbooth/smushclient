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
  QTextCharFormat charFormat() const { return noteFormat; }
  void clear();
  QTextDocument* document() const { return cursor.document(); }
  void echo(const QString& text);
  void finishNote();
  void mergeCharFormat(const QTextCharFormat& format);
  void move(QTextCursor::MoveOperation op, int count);
  void setIndentText(const QString& text) noexcept;
  void setSuppressingEcho(bool suppress = true) noexcept;
  int startLine();
  bool suppressingEcho() const noexcept { return m_suppressingEcho; }
  void setOption(std::string_view name, int64_t value);
  void updateTimestamp();

  explicit operator QTextCursor() const { return cursor; }

signals:
  void noteLogged(const QString& note);

private:
  void flushLine();
  void insertBlock();

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
  bool m_suppressingEcho = false;
  QTextCharFormat noteFormat;
};
