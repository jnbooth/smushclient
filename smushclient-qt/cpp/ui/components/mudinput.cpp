#include "mudinput.h"
#include <QtGui/QKeyEvent>
#include <QtGui/QTextBlock>
#include <QtGui/QTextDocument>

// Public methods

MudInput::MudInput(QWidget *parent)
    : QTextEdit(parent) {}

MudInput::MudInput(QWidget *parent, const QStringList &history)
    : QTextEdit(parent),
      history(history) {}

MudInput::MudInput(const QStringList &history)
    : QTextEdit(),
      history(history) {}

void MudInput::clearLog()
{
  history.clear();
}

const QStringList &MudInput::log() const noexcept
{
  return history.log();
}

void MudInput::remember(const QString &text)
{
  history.push(text);
}

void MudInput::setIgnoreKeypad(bool ignore)
{
  ignoreKeypad = ignore;
}

void MudInput::setLog(const QStringList &log)
{
  history.replace(log);
}

void MudInput::setMaxLogSize(qsizetype size)
{
  history.setMaxSize(size);
}

// Public overrides

QSize MudInput::minimumSizeHint() const
{
  const QFontMetrics metrics = fontMetrics();
  return QSize(metrics.maxWidth(), metrics.lineSpacing() + 8);
}

QSize MudInput::sizeHint() const
{
  return minimumSizeHint();
}

// Protected overrides

void MudInput::keyPressEvent(QKeyEvent *event)
{
  if (ignoreKeypad && event->modifiers().testFlag(Qt::KeypadModifier)) [[unlikely]]
  {
    event->ignore();
    return;
  }
  switch (event->key())
  {
  case Qt::Key::Key_Up:
    if (int lines = document()->lineCount(); lines > 1 && !textCursor().atStart())
      break;
    setTextFromHistory(history.previous());
    return;

  case Qt::Key::Key_Down:
    if (int lines = document()->lineCount(); lines > 1 && !textCursor().atEnd())
      break;
    setTextFromHistory(history.next());
    return;

  case Qt::Key::Key_Enter:
  case Qt::Key::Key_Return:
    if (event->modifiers().testFlag(Qt::KeyboardModifier::ShiftModifier))
    {
      QTextEdit::keyPressEvent(event);
      return;
    }
    const QString text = toPlainText();
    emit submitted(text);
    return;
  }

  QTextEdit::keyPressEvent(event);
}

// Private methods

void MudInput::restoreDraft()
{
  if (draft.isEmpty())
    return;
  setText(draft);
  moveCursor(QTextCursor::MoveOperation::End);
  draft.clear();
}

void MudInput::saveDraft()
{
  if (!history.atEnd() || document()->isEmpty())
    return;
  draft = toPlainText();
}

void MudInput::setTextFromHistory(const QString &text)
{
  if (text.isEmpty())
  {
    restoreDraft();
    return;
  }
  saveDraft();
  setText(text);
  moveCursor(QTextCursor::MoveOperation::End);
}
