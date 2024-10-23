#include "mudinput.h"
#include <QtGui/QKeyEvent>

// Public methods

MudInput::MudInput(QWidget *parent)
    : QLineEdit(parent),
      draft(),
      history() {}

MudInput::MudInput(QWidget *parent, const QStringList &history)
    : QLineEdit(parent),
      draft(),
      history(history) {}

MudInput::MudInput(const QStringList &history)
    : QLineEdit(),
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

// Protected overrides

void MudInput::keyPressEvent(QKeyEvent *event)
{
  switch (event->key())
  {
  case Qt::Key::Key_Up:
    previous();
    return;

  case Qt::Key::Key_Down:
    next();
    return;

  case Qt::Key::Key_Enter:
  case Qt::Key::Key_Return:
    if (const QString input = text(); !input.isEmpty())
      history.push(input);
  }

  QLineEdit::keyPressEvent(event);
}

// Private methods

void MudInput::previous()
{
  const int pos = cursorPosition();
  home(false);
  if (cursorPosition() != pos)
    return;

  setTextFromHistory(history.previous());
}

void MudInput::next()
{
  const int pos = cursorPosition();
  end(false);
  if (cursorPosition() != pos)
    return;

  setTextFromHistory(history.next());
}

void MudInput::restoreDraft()
{

  if (!draft.isEmpty())
  {
    setText(draft);
    setModified(true);
    draft.clear();
  }
}

void MudInput::saveDraft()
{
  if (!isModified())
    return;
  const QString value = text();
  if (value.isEmpty())
    return;
  draft = value;
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
