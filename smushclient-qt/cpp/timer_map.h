#pragma once
#include <QtCore/QMetaObject>
#include <QtCore/QObject>
#include <QtCore/QTimerEvent>

template<typename T>
class TimerMap : public QObject
{
private:
  template<typename Handler>
  using HandlerSlot = bool (Handler::*)(const T& item);

public:
  template<typename Parent>
  inline TimerMap(Parent* parent,
                  HandlerSlot<Parent> slot,
                  Qt::TimerType timerType = Qt::TimerType::CoarseTimer)
    : QObject(parent)
    , slot((HandlerSlot<QObject>)slot)
    , timerType(timerType)
  {
  }

  inline void clear() noexcept
  {
    if (map.empty())
      return;

    for (const auto& entry : map)
      killTimer(entry.first);
    map.clear();
  }

  template<class Predicate>
  inline size_t erase_if(Predicate pred)
  {
    return std::erase_if(map,
                         [pred](const std::pair<const Qt::TimerId, T>& item) {
                           return pred(item.second);
                         });
  }

  template<class... Args>
  inline T& start(std::chrono::milliseconds duration, Args&&... args)
  {
    return map.emplace(startTimerId(duration), std::forward<Args>(args)...)
      .second;
  }

  inline T& start(std::chrono::milliseconds duration, T item)
  {
    return map[startTimerId(duration)] = item;
  }

protected:
  inline void timerEvent(QTimerEvent* event) override
  {
    const Qt::TimerId id = event->id();
    auto search = map.find(id);
    if (search == map.end()) [[unlikely]]
      return;
    if (!(*parent().*slot)(search->second))
      return;
    killTimer(id);
    map.erase(search);
  }

private:
  inline Qt::TimerId startTimerId(std::chrono::milliseconds duration)
  {
    return Qt::TimerId{ startTimer(duration, timerType) };
  }

private:
  std::unordered_map<Qt::TimerId, T> map{};
  HandlerSlot<QObject> slot;
  Qt::TimerType timerType;
};
