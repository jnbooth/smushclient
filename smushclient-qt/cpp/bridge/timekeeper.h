#ifndef TIMEKEEPER_H
#define TIMEKEEPER_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QTimerEvent>

class ScriptApi;
enum class SendTarget;
struct SendTimer;

class Timekeeper : public QObject
{
  Q_OBJECT

private:
  struct Item
  {
    size_t index;
    uint16_t timerId;
  };

public:
  Timekeeper(ScriptApi *parent);
  void beginPolling(std::chrono::milliseconds interval, Qt::TimerType timerType = Qt::CoarseTimer);
  void cancelTimers(const QSet<uint16_t> &timerIds);
  void sendTimer(const SendTimer &timer) const;
  void startSendTimer(size_t index, uint16_t timerId, uint milliseconds);
  inline void setOpen(bool open) { closed = !open; }

protected:
  void timerEvent(QTimerEvent *event) override;

private:
  bool closed = true;
  int pollTimerId = -1;
  std::unordered_map<int, Item> queue{};

  ScriptApi *getApi() const;
};

#endif // Timekeeper
