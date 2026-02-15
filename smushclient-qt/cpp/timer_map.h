#pragma once
#include <QtCore/QPointer>
#include <QtCore/QTimerEvent>

template<typename T, typename Handler>
class TimerMap : public QObject
{
private:
  using HandlerSlot = bool (Handler::*)(const T& item);

public:
  TimerMap(Handler& handler,
           HandlerSlot slot,
           Qt::TimerType timerType,
           QObject* parent = nullptr)
    : QObject(parent)
    , handler(&handler)
    , slot(slot)
    , timerType(timerType)
  {
  }

  TimerMap(Handler& handler, HandlerSlot slot, QObject* parent = nullptr)
    : QObject(parent)
    , handler(&handler)
    , slot(slot)
    , timerType(Qt::TimerType::CoarseTimer)
  {
  }

  void clear() noexcept
  {
    if (map.empty()) {
      return;
    }

    for (const auto& entry : map) {
      killTimer(entry.first);
    }
    map.clear();
  }

  template<class Predicate>
  size_t erase_if(Predicate pred)
  {
    return std::erase_if(map,
                         [pred](const std::pair<const Qt::TimerId, T>& item) {
                           return pred(item.second);
                         });
  }

  template<class... Args>
  T& start(std::chrono::milliseconds duration, Args&&... args)
  {
    return map.emplace(startTimerId(duration), std::forward<Args>(args)...)
      .second;
  }

  T& start(std::chrono::milliseconds duration, T item)
  {
    return map[startTimerId(duration)] = item;
  }

protected:
  void timerEvent(QTimerEvent* event) override
  {
    const Qt::TimerId id = event->id();
    auto search = map.find(id);
    if (search == map.end()) [[unlikely]] {
      return;
    }
    if (handler != nullptr && !(*handler.*slot)(search->second)) {
      return;
    }
    killTimer(id);
    map.erase(search);
  }

private:
  Qt::TimerId startTimerId(std::chrono::milliseconds duration)
  {
    return Qt::TimerId{ startTimer(duration, timerType) };
  }

private:
  QPointer<Handler> handler;
  std::unordered_map<Qt::TimerId, T> map;
  HandlerSlot slot;
  Qt::TimerType timerType;
};
