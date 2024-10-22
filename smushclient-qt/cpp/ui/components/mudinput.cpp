#include "mudinput.h"
#include <QtGui/QKeyEvent>

// Public methods

MudInput::MudInput(QWidget *parent)
    : QLineEdit(parent),
      history() {}

MudInput::MudInput(QWidget *parent, const QStringList &history)
    : QLineEdit(parent),
      history(history) {}

MudInput::MudInput(const QStringList &history)
    : QLineEdit(),
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
    if (const QString &historyEntry = history.previous(); !historyEntry.isEmpty())
      setText(historyEntry);
    return;

  case Qt::Key::Key_Down:
    setText(history.next());
    return;

  case Qt::Key::Key_Enter:
  case Qt::Key::Key_Return:
    if (const QString input = text(); !input.isEmpty())
      history.push(input);
  }

  QLineEdit::keyPressEvent(event);
}
