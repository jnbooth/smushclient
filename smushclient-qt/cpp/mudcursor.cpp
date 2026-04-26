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
MudCursor::setIndentText(const QString& text) noexcept
{
  indentText = text;
}

void
MudCursor::setSuppressingEcho(bool suppress) noexcept
{
  m_suppressingEcho = suppress;
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
  cursor.insertText(text, format);
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
  cursor.insertText(text, format);
}

void
MudCursor::applyWorld(const World& world)
{
  logNotes = world.getLogNotes();
  echoOnSameLine = world.getKeepCommandsOnSameLine();

  if (world.getNoEchoOff()) {
    m_suppressingEcho = false;
  }

  indentText = QString(world.getIndentParas(), u' ');
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

void
MudCursor::echo(const QString& text)
{
  if (m_suppressingEcho) [[unlikely]] {
    return;
  }
  if (echoOnSameLine) {
    cursor.insertText(text, echoFormat);
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
MudCursor::mergeCharFormat(const QTextCharFormat& format)
{
  noteFormat.merge(format);
}

void
MudCursor::move(QTextCursor::MoveOperation op, int count)
{
  using MoveMode = QTextCursor::MoveMode;
  using MoveOperation = QTextCursor::MoveOperation;

  if (count <= 0) {
    return;
  }
  flushLine();
  switch (op) {
    case MoveOperation::Up:
      cursor.movePosition(MoveOperation::Up, MoveMode::KeepAnchor, count);
      cursor.removeSelectedText();
      break;
    case MoveOperation::Left:
      cursor.movePosition(MoveOperation::Left,
                          MoveMode::KeepAnchor,
                          std::min(count, cursor.positionInBlock()));
      cursor.removeSelectedText();
      break;
    case MoveOperation::Down: {
      const int column = cursor.positionInBlock();
      for (; count != 0; --count) {
        if (!cursor.movePosition(
              MoveOperation::Down, MoveMode::MoveAnchor, count)) {
          break;
        }
      }
      for (int i = 0; i < count; ++i) {
        cursor.insertBlock();
      }
      const int columnOffset = column - cursor.positionInBlock();
      if (columnOffset > 0) {
        cursor.insertText(QString(columnOffset, u' '), QTextCharFormat());
      }
      break;
    }
    case MoveOperation::Right: {
      if (!cursor.atBlockEnd()) {
        const QTextBlock block = cursor.block();
        const int offset = block.length() - cursor.positionInBlock();
        if (count <= offset) {
          cursor.movePosition(
            MoveOperation::Right, MoveMode::MoveAnchor, count);
          break;
        }
        cursor.movePosition(MoveOperation::EndOfBlock, MoveMode::MoveAnchor);
        count -= offset;
      }
      cursor.insertText(QString(count, u' '), QTextCharFormat());
      break;
    }
    default:
      cursor.movePosition(op, MoveMode::KeepAnchor, count);
      break;
  }
  if (cursor.atEnd() && cursor.atBlockStart()) {
    cursor.movePosition(MoveOperation::PreviousCharacter, MoveMode::KeepAnchor);
    cursor.movePosition(MoveOperation::EndOfBlock, MoveMode::KeepAnchor);
    cursor.removeSelectedText();
    hasLine = true;
  }
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
    indentText = QString(value, u' ');
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
      m_suppressingEcho = false;
    }
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
  cursor.insertText(indentText);
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
