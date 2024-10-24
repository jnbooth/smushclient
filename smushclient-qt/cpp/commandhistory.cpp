#include "commandhistory.h"

const static QString emptyString;

// Public methods

CommandHistory::CommandHistory(qsizetype max)
    : history(),
      max(max),
      begin(history.cbegin()),
      iterator(history.cend()),
      last(iterator - 1) {}

CommandHistory::CommandHistory(const QStringList &borrowHistory, qsizetype max)
    : history(borrowHistory),
      max(max),
      begin(history.cbegin()),
      iterator(history.cend()),
      last(iterator - 1) {}

CommandHistory::CommandHistory(CommandHistory &&other)
    : CommandHistory(std::move(other.history), other.max) {}

CommandHistory::CommandHistory(const CommandHistory &other)
    : CommandHistory(other.history, other.max) {}

CommandHistory &CommandHistory::operator=(const CommandHistory &other)
{
  history = other.history;
  max = other.max;
  resetIterators();
  return *this;
}

CommandHistory &CommandHistory::operator=(const QStringList &other)
{
  history = other;
  max = SSIZE_MAX;
  resetIterators();
  return *this;
}

void CommandHistory::clear()
{
  history.clear();
  resetIterators();
}

void CommandHistory::pop() noexcept
{
  history.pop_back();
  resetIterators();
}

void CommandHistory::push(const QString &command)
{
  const qsizetype currentSize = size();
  if (currentSize && *last == command)
    return;

  if (currentSize == max)
    history.removeFirst();

  history.append(command);

  resetIterators();
}

void CommandHistory::replace(const QStringList &newHistory)
{
  const qsizetype newSize = newHistory.size();
  history = newSize <= max ? newHistory : newHistory.sliced(newSize - max, max);
  resetIterators();
}

void CommandHistory::setMaxSize(qsizetype newMax)
{
  max = newMax;
  const qsizetype currentSize = size();
  if (currentSize <= max)
    return;

  history.remove(0, currentSize - max);
  resetIterators();
}

// Private functions

void CommandHistory::resetIterators() noexcept
{
  begin = history.cbegin();
  iterator = history.cend();
  last = iterator - 1;
}
