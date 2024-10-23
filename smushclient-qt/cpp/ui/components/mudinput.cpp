#include "mudinput.h"
#include <QtGui/QKeyEvent>

// Public methods

MudInput::MudInput(QWidget *parent)
    : QTextEdit(parent),
      draft(),
      history() {}

MudInput::MudInput(QWidget *parent, const QStringList &history)
    : QTextEdit(parent),
      draft(),
      history(history) {}

MudInput::MudInput(const QStringList &history)
    : QTextEdit(),
      draft(),
      history(history) {}

void MudInput::clearLog()
{
  history.clear();
}

void MudInput::forgetLast() noexcept
{
  history.pop();
}

const QStringList &MudInput::log() const noexcept
{
  return history.log();
}

void MudInput::remember(const QString &text)
{
  history.push(text);
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
  switch (event->key())
  {
  case Qt::Key::Key_Up:
    if (textCursor().atStart())
    {
      setTextFromHistory(history.previous());
      return;
    }

  case Qt::Key::Key_Down:
    if (textCursor().atEnd())
    {
      setTextFromHistory(history.next());
      return;
    }

  case Qt::Key::Key_Enter:
  case Qt::Key::Key_Return:
    if (event->modifiers().testFlag(Qt::KeyboardModifier::ShiftModifier))
    {
      QTextEdit::keyPressEvent(event);
      return;
    }
    if (const QString text = toPlainText(); !text.isEmpty())
    {
      history.push(text);
      emit submitted(text);
      clear();
      return;
    }
  }

  QTextEdit::keyPressEvent(event);
}

// Private methods

void MudInput::restoreDraft()
{
  if (!draft.isEmpty())
  {
    setText(draft);
    draft.clear();
  }
}

void MudInput::saveDraft()
{
  if (document()->isEmpty())
    return;
  const QString text = toPlainText();
  if (text == history.current())
    return;
  draft = text;
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
}
