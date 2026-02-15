#ifndef TIMEKEEPER_H
#define TIMEKEEPER_H

#include <QtCore/QTimer>

struct SendTimer;
class SmushClient;
template<typename T>
class TimerMap;

struct TimekeeperItem
{
  size_t index;
  uint16_t timerId;
};

class Timekeeper : public QObject
{
  Q_OBJECT

public:
  using Item = TimekeeperItem;

public:
  explicit Timekeeper(SmushClient& client, QObject* parent = nullptr);
  void beginPolling(std::chrono::milliseconds interval,
                    Qt::TimerType timerType = Qt::TimerType::CoarseTimer);
  void cancelTimers(const QSet<uint16_t>& timerIds);
  void sendTimer(const SendTimer& timer) const;
  void startSendTimer(size_t index,
                      uint16_t timerId,
                      unsigned int millis) const;

private:
  bool finishTimer(const Item& item);

private:
  QTimer* pollTimer;
  TimerMap<Item>* queue;
};

#endif // Timekeeper
