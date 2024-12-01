#include "commandhistory.h"
#include <limits>

const static QString emptyString;

// Public methods

CommandHistory::CommandHistory(qsizetype max)
    : history(),
      max(max),
      begin(history.cbegin()),
      end(history.cend()),
      iterator(end) {}

CommandHistory::CommandHistory(const QStringList &borrowHistory, qsizetype max)
    : history(borrowHistory),
      max(max),
      begin(history.cbegin()),
      end(history.cend()),
      iterator(end) {}

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

bool CommandHistory::push(const QString &command)
{
  const qsizetype currentSize = size();
  if (currentSize && *(end - 1) == command)
    return false;

  if (currentSize == max)
    history.removeFirst();

  history.append(command);

  resetIterators();

  return true;
}

void CommandHistory::replace(const QStringList &newHistory)
{
  const qsizetype newSize = newHistory.size();
  history = newSize <= max ? newHistory : newHistory.sliced(newSize - max, max);
  resetIterators();
}

void CommandHistory::setMaxSize(qsizetype newMax)
{
  if (newMax < 0)
  {
    max = SSIZE_MAX;
    return;
  }

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
  end = history.cend();
  iterator = end;
}
