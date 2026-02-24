#pragma once
#include <QtCore/QStringList>

class CommandHistory
{
public:
  explicit CommandHistory(qsizetype max = -1) noexcept;
  explicit CommandHistory(const QStringList& history, qsizetype max = -1);
  CommandHistory(CommandHistory&& other) noexcept;
  CommandHistory(const CommandHistory& other);
  ~CommandHistory() = default;

  CommandHistory& operator=(const CommandHistory& other);
  CommandHistory& operator=(const QStringList& other);
  CommandHistory& operator=(CommandHistory&&) = delete;

  void clear();
  void pop();
  bool push(const QString& command);
  void replace(const QStringList& history);
  void setMaxSize(qsizetype max);

  const QString& previous() noexcept
  {
    return iterator == begin ? _emptyString : *--iterator;
  }

  const QString& current() const noexcept
  {
    return iterator == end ? _emptyString : *iterator;
  }

  const QString& next() noexcept
  {
    return iterator == end || (++iterator == end) ? _emptyString : *iterator;
  }

  bool atStart() const noexcept { return iterator == begin; }

  bool atLast() const noexcept { return iterator == end - 1; }

  bool atEnd() const noexcept { return iterator == end; }

  bool isEmpty() const noexcept { return begin == end; }

  bool isFull() const noexcept { return size() == max; }

  const QStringList& log() const noexcept { return history; }

  qsizetype maxSize() const noexcept { return max; }

  qsizetype size() const noexcept { return end - begin; }

private:
  void resetIterators();

private:
  static const QString _emptyString;
  QStringList history;
  qsizetype max;
  QStringList::const_iterator begin;
  QStringList::const_iterator end;
  QStringList::const_iterator iterator;
};
