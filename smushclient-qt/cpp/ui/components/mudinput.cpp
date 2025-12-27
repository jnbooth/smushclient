#include "mudinput.h"
#include <QtGui/QKeyEvent>
#include <QtGui/QTextBlock>
#include <QtGui/QTextDocument>

// Public methods

MudInput::MudInput(QWidget* parent)
  : QTextEdit(parent)
  , history()
{
}

MudInput::MudInput(QWidget* parent, const QStringList& history)
  : QTextEdit(parent)
  , history(history)
{
}

MudInput::MudInput(const QStringList& history)
  : QTextEdit()
  , history(history)
{
}

const QStringList&
MudInput::log() const noexcept
{
  return history.log();
}

void
MudInput::setLog(const QStringList& log)
{
  history.replace(log);
}

// Public overrides

QSize
MudInput::minimumSizeHint() const
{
  const QFontMetrics metrics = fontMetrics();
  return QSize(metrics.maxWidth(), metrics.lineSpacing() + 8);
}

QSize
MudInput::sizeHint() const
{
  return minimumSizeHint();
}

// Public slots

void
MudInput::clearLog()
{
  history.clear();
}

void
MudInput::remember(const QString& text)
{
  history.push(text);
}

void
MudInput::setIgnoreKeypad(bool ignore)
{
  ignoreKeypad = ignore;
}

void
MudInput::setMaxLogSize(int size)
{
  history.setMaxSize(size);
}

// Protected overrides

void
MudInput::keyPressEvent(QKeyEvent* event)
{
  switch (event->key()) {
    case Qt::Key::Key_Up:
      if (history.atStart())
        break;
      if (int lines = document()->lineCount();
          lines > 1 && !textCursor().atStart())
        break;
      if ((history.atEnd() || history.atLast()) && !document()->isEmpty() &&
          history.push(toPlainText()))
        history.previous();
      setText(history.previous());
      moveCursor(QTextCursor::MoveOperation::End);
      return;

    case Qt::Key::Key_Down:
      if (int lines = document()->lineCount();
          lines > 1 && !textCursor().atEnd())
        break;
      setText(history.next());
      moveCursor(QTextCursor::MoveOperation::End);
      return;

    case Qt::Key::Key_Enter:
    case Qt::Key::Key_Return: {
      if (event->modifiers().testFlag(Qt::KeyboardModifier::ShiftModifier)) {
        QTextEdit::keyPressEvent(event);
        return;
      }
      const QString text = toPlainText();
      emit submitted(text);
      return;
    }

    case Qt::Key::Key_Left:
    case Qt::Key::Key_Right:
      break;

    default:
      if (ignoreKeypad && event->modifiers().testFlag(Qt::KeypadModifier))
        [[unlikely]] {
        event->ignore();
        return;
      }
  }

  QTextEdit::keyPressEvent(event);
}
