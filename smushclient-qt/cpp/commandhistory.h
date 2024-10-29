#pragma once
#include <QtCore/QStringList>

const static QString __emptyString;

class CommandHistory
{
public:
  explicit CommandHistory(qsizetype max = SSIZE_MAX);
  explicit CommandHistory(const QStringList &history, qsizetype max = SSIZE_MAX);
  CommandHistory(CommandHistory &&other);
  CommandHistory(const CommandHistory &other);
  CommandHistory &operator=(const CommandHistory &other);
  CommandHistory &operator=(const QStringList &history);

  void clear();
  void pop() noexcept;
  bool push(const QString &command);
  void replace(const QStringList &history);
  void setMaxSize(qsizetype max);

  constexpr const QString &previous() noexcept
  {
    return iterator == begin ? __emptyString : *--iterator;
  }

  constexpr const QString &current() const noexcept
  {
    return iterator == end ? __emptyString : *iterator;
  }

  constexpr const QString &next() noexcept
  {
    return iterator == end || (++iterator == end) ? __emptyString : *iterator;
  }

  constexpr bool atStart() const noexcept
  {
    return iterator == begin;
  }

  constexpr bool atLast() const noexcept
  {
    return iterator == end - 1;
  }

  constexpr bool atEnd() const noexcept
  {
    return iterator == end;
  }

  constexpr bool isEmpty() const noexcept
  {
    return size() == 0;
  }

  constexpr bool isFull() const noexcept
  {
    return size() == max;
  }

  constexpr const QStringList &log() const noexcept
  {
    return history;
  }

  constexpr qsizetype maxSize() const noexcept
  {
    return max;
  }

  constexpr qsizetype size() const noexcept
  {
    return end - begin;
  }

private:
  QStringList history;
  qsizetype max;
  QStringList::const_iterator begin;
  QStringList::const_iterator end;
  QStringList::const_iterator iterator;

private:
  void resetIterators() noexcept;
};
