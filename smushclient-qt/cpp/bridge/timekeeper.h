#ifndef TIMEKEEPER_H
#define TIMEKEEPER_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QTimerEvent>

class ScriptApi;
enum class SendTarget : uint8_t;
struct SendTimer;

class Timekeeper : public QObject
{
  Q_OBJECT

public:
  Timekeeper(ScriptApi *parent);
  void sendTimer(const SendTimer &timer) const;
  void startSendTimer(size_t id, uint milliseconds);
  inline void setOpen(bool open) { closed = !open; }

protected:
  void timerEvent(QTimerEvent *event) override;

private:
  bool closed;
  std::unordered_map<int, size_t> queue;

  ScriptApi *getApi() const;
};

#endif // Timekeeper
