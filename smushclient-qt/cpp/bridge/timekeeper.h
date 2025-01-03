#ifndef TIMEKEEPER_H
#define TIMEKEEPER_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QTimer>

class ScriptApi;
enum class SendTarget;
struct SendTimer;
template <typename T>
class TimerMap;

class Timekeeper : public QObject
{
  Q_OBJECT

public:
  Timekeeper(ScriptApi *parent);
  void beginPolling(std::chrono::milliseconds interval, Qt::TimerType timerType = Qt::CoarseTimer);
  void cancelTimers(const QSet<uint16_t> &timerIds);
  void sendTimer(const SendTimer &timer) const;
  void startSendTimer(size_t index, uint16_t timerId, uint milliseconds);
  inline void setOpen(bool open) { closed = !open; }

private:
  struct Item
  {
    size_t index;
    uint16_t timerId;
  };

private:
  bool finishTimer(const Item &item);
  ScriptApi *getApi() const;

private slots:
  void pollTimers();

private:
  bool closed = true;
  QTimer *pollTimer;
  TimerMap<Item> *queue;
};

#endif // Timekeeper
