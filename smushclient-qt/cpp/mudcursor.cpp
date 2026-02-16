#include "mudcursor.h"
#include "smushclient_qt/src/ffi/world.cxxqt.h"
#include "spans.h"

#include <QtGui/QTextBlock>

using std::string_view;

// Public methods

MudCursor::MudCursor(QTextDocument* document)
  : QObject(document)
  , cursor(document)
{
  spans::setLineType(echoFormat, LineType::Input);
  spans::setLineType(noteFormat, LineType::Note);
}

void
MudCursor::setIndentText(const QString& text)
{
  indentText = text;
}

void
MudCursor::setSuppressingEcho(bool suppress)
{
  suppressEcho = suppress;
}

void
MudCursor::appendError(const QString& message)
{
  appendText(message, errorFormat);
  startLine();
}

void
MudCursor::appendHtml(const QString& html)
{
  flushLine();
  cursor.insertHtml(html);
}

void
MudCursor::appendTell(const QString& text, const QTextCharFormat& format)
{
  if (text.isEmpty()) {
    return;
  }
  if (cursor.position() > lastTellPosition) {
    flushLine();
    updateTimestamp();
  }
  insertText(text, format);
  hasLine = true;
  lastTellPosition = cursor.position();
  if (logNotes) {
    emit noteLogged(text);
  }
}

void
MudCursor::appendText(const QString& text, const QTextCharFormat& format)
{
  flushLine();
  insertText(text, format);
}

void
MudCursor::applyWorld(const World& world)
{
  logNotes = world.getLogNotes();
  echoOnSameLine = world.getKeepCommandsOnSameLine();

  if (world.getNoEchoOff()) {
    suppressEcho = false;
  }

  indentText = QStringLiteral(" ").repeated(world.getIndentParas());
  echoFormat.setForeground(world.getEchoColour());
  echoFormat.setBackground(world.getEchoBackgroundColour());
  errorFormat.setForeground(world.getErrorTextColour());
  errorFormat.setBackground(world.getErrorBackgroundColour());
  noteFormat.setForeground(world.getNoteTextColour());
  noteFormat.setBackground(world.getNoteBackgroundColour());
}

void
MudCursor::clear()
{
  cursor.document()->clear();
  hasLine = false;
  indentNext = false;
  lastLinePosition = -1;
  lastTellPosition = -1;
}

QTextDocument*
MudCursor::document() const
{
  return cursor.document();
}

void
MudCursor::echo(const QString& text)
{
  if (suppressEcho) [[unlikely]] {
    return;
  }
  if (echoOnSameLine) {
    insertText(text, echoFormat);
    return;
  }
  appendText(text, echoFormat);
  startLine();
}

void
MudCursor::finishNote()
{
  lastTellPosition = -1;
}

void
MudCursor::move(QTextCursor::MoveOperation op, int count)
{
  switch (op) {
    case QTextCursor::MoveOperation::Right: {
      const int pos = cursor.position();
      cursor.movePosition(op, QTextCursor::MoveMode::MoveAnchor, count);
      count -= cursor.position() - pos;
      if (count == 0) {
        break;
      }
      flushLine();
      insertText(QStringLiteral(" ").repeated(count), QTextCharFormat());
      break;
    }
    case QTextCursor::MoveOperation::Down: {
      const int position = cursor.blockNumber();
      cursor.movePosition(op, QTextCursor::MoveMode::MoveAnchor, count);
      count -= cursor.blockNumber() - position;
      if (count == 0) {
        break;
      }
      for (int i = 0; i < count; ++i) {
        startLine();
      }
      if (position != 0) {
        flushLine();
        insertText(QStringLiteral(" ").repeated(count), QTextCharFormat());
      }
      break;
    }
    default:
      cursor.movePosition(op, QTextCursor::MoveMode::KeepAnchor, count);
      break;
  }
}

int
MudCursor::startLine()
{
  if (hasLine) [[unlikely]] {
    insertBlock();
    indentNext = !indentText.isEmpty();
  } else {
    hasLine = true;
  }
  return cursor.position();
}

void
MudCursor::setOption(string_view name, int64_t value)
{
  if (name == "echo_colour") {
    echoFormat.setForeground(QColor(value));
  } else if (name == "echo_background_colour") {
    echoFormat.setBackground(QColor(value));
  } else if (name == "error_text_colour") {
    errorFormat.setForeground(QColor(value));
  } else if (name == "error_background_colour") {
    errorFormat.setBackground(QColor(value));
  } else if (name == "indent_paras") {
    indentText = QStringLiteral(" ").repeated(value);
  } else if (name == "keep_commands_on_same_line") {
    echoOnSameLine = value == 1;
  } else if (name == "log_notes") {
    logNotes = value == 1;
  } else if (name == "note_text_colour") {
    noteFormat.setForeground(QColor(value));
  } else if (name == "note_background_colour") {
    noteFormat.setBackground(QColor(value));
  } else if (name == "no_echo_off") {
    if (value == 1) {
      suppressEcho = false;
    }
  }
}

void
MudCursor::updateTimestamp()
{
  spans::setTimestamp(cursor);
}

// Private methods

void
MudCursor::flushLine()
{
  if (!hasLine) [[likely]] {
    return;
  }

  hasLine = false;
  insertBlock();

  if (!indentNext) [[likely]] {
    return;
  }

  indentNext = false;
  insertText(indentText);
}

void
MudCursor::insertBlock()
{
  if (logNotes && lastTellPosition >= lastLinePosition) {
    emit noteLogged(cursor.block().text());
  }
  cursor.insertBlock();
  lastLinePosition = cursor.position();
}

void
MudCursor::insertText(const QString& text, const QTextCharFormat& format)
{
  if (!cursor.atBlockEnd()) {
    cursor.movePosition(QTextCursor::MoveOperation::NextCharacter,
                        QTextCursor::MoveMode::KeepAnchor,
                        static_cast<int>(text.size()));
    cursor.removeSelectedText();
  }
  cursor.insertText(text, format);
}
