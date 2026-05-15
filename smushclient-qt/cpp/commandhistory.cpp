#include "commandhistory.h"
#include <limits>

constexpr qsizetype maxLimit = std::numeric_limits<qsizetype>::max();

const QString CommandHistory::_emptyString = QString();

// Private utils

namespace {
constexpr qsizetype
adjustMax(qsizetype max) noexcept
{
  return max == -1 ? maxLimit : max;
}
} // namespace

CommandHistory::CommandHistory(qsizetype max) noexcept
  : max(adjustMax(max))
  , begin(history.cbegin())
  , end(history.cend())
  , iterator(end)
{
}

CommandHistory::CommandHistory(const QStringList& borrowHistory,
                               qsizetype max) noexcept
  : history(borrowHistory)
  , max(adjustMax(max))
  , begin(history.cbegin())
  , end(history.cend())
  , iterator(end)
{
}

CommandHistory::CommandHistory(CommandHistory&& other) noexcept
  : history(std::move(other.history))
  , max(other.max)
  , begin(history.cbegin())
  , end(history.cend())
  , iterator(end)
{
}

CommandHistory::CommandHistory(const CommandHistory& other) noexcept
  : CommandHistory(other.history, other.max)
{
}

CommandHistory&
CommandHistory::operator=(const CommandHistory& other) noexcept
{
  history = other.history;
  max = other.max;
  resetIterators();
  return *this;
}

CommandHistory&
CommandHistory::operator=(const QStringList& other) noexcept
{
  history = other;
  max = maxLimit;
  resetIterators();
  return *this;
}

void
CommandHistory::clear()
{
  history.clear();
  resetIterators();
}

void
CommandHistory::pop() noexcept
{
  history.pop_back();
  resetIterators();
}

bool
CommandHistory::push(const QString& command)
{
  const qsizetype currentSize = size();
  if ((currentSize != 0) && *(end - 1) == command) {
    return false;
  }

  if (currentSize == max) {
    history.removeFirst();
  }

  history.append(command);

  resetIterators();

  return true;
}

void
CommandHistory::setMaxSize(qsizetype newMax)
{
  if (newMax < 0) {
    max = maxLimit;
    return;
  }

  max = newMax;
  const qsizetype currentSize = size();
  if (currentSize <= max) {
    return;
  }

  history.remove(0, currentSize - max);
  resetIterators();
}

// Private functions

void
CommandHistory::resetIterators() noexcept
{
  begin = history.cbegin();
  end = history.cend();
  iterator = end;
}
