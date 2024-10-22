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
  void push(const QString &command);
  void replace(const QStringList &history);
  void setMaxSize(qsizetype max);

  constexpr const QString &previous() noexcept
  {
    return iterator == begin ? __emptyString : *--iterator;
  }

  constexpr const QString &next() noexcept
  {
    return iterator >= last ? __emptyString : *++iterator;
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
    return 1 + (last - begin);
  }

private:
  QStringList history;
  qsizetype max;
  QStringList::const_iterator begin;
  QStringList::const_iterator iterator;
  QStringList::const_iterator last;

private:
  void resetIterators() noexcept;
};
