#pragma once
#include <QtCore/QStringList>

#ifndef SSIZE_MAX
#include <limits>
constexpr auto SSIZE_MAX = std::numeric_limits<qsizetype>::max();
#endif

class CommandHistory
{
public:
  explicit CommandHistory(qsizetype max = SSIZE_MAX);
  explicit CommandHistory(const QStringList& history,
                          qsizetype max = SSIZE_MAX);
  CommandHistory(CommandHistory&& other) noexcept;
  CommandHistory(const CommandHistory& other);
  ~CommandHistory() = default;

  CommandHistory& operator=(const CommandHistory& other);
  CommandHistory& operator=(const QStringList& other);
  CommandHistory& operator=(CommandHistory&&) = delete;

  void clear();
  void pop() noexcept;
  bool push(const QString& command);
  void replace(const QStringList& history);
  void setMaxSize(qsizetype max);

  constexpr const QString& previous() noexcept
  {
    return iterator == begin ? _emptyString : *--iterator;
  }

  constexpr const QString& current() const noexcept
  {
    return iterator == end ? _emptyString : *iterator;
  }

  constexpr const QString& next() noexcept
  {
    return iterator == end || (++iterator == end) ? _emptyString : *iterator;
  }

  constexpr bool atStart() const noexcept { return iterator == begin; }

  constexpr bool atLast() const noexcept { return iterator == end - 1; }

  constexpr bool atEnd() const noexcept { return iterator == end; }

  constexpr bool isEmpty() const noexcept { return begin == end; }

  bool isFull() const noexcept { return size() == max; }

  constexpr const QStringList& log() const noexcept { return history; }

  constexpr qsizetype maxSize() const noexcept { return max; }

  qsizetype size() const noexcept { return end - begin; }

private:
  void resetIterators() noexcept;

private:
  static const QString _emptyString;
  QStringList history;
  qsizetype max;
  QStringList::const_iterator begin;
  QStringList::const_iterator end;
  QStringList::const_iterator iterator;
};
