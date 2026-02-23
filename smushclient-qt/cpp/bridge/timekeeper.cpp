#include "timekeeper.h"
#include "../client.h"
#include "../scripting/scriptapi.h"
#include "../timer_map.h"
#include "smushclient_qt/src/ffi/timekeeper.cxxqt.h"

using std::chrono::milliseconds;
using std::chrono::seconds;

Timekeeper::Timekeeper(SmushClient& client, QObject* parent)
  : QObject(parent)
  , pollTimer(new QTimer(this))
  , queue(new TimerMap<Timekeeper::Item, SmushClient>(client,
                                                      &SmushClient::finishTimer,
                                                      this))
{
  connect(pollTimer, &QTimer::timeout, &client, &SmushClient::onTimersPolled);
}

void
Timekeeper::beginPolling(milliseconds interval, Qt::TimerType timerType)
{
  pollTimer->setInterval(interval);
  pollTimer->setTimerType(timerType);
  pollTimer->start();
}

void
Timekeeper::cancelTimers(const QSet<uint16_t>& timerIds)
{
  queue->erase_if([timerIds](const Timekeeper::Item& item) {
    return timerIds.contains(item.timerId);
  });
}

void
Timekeeper::startSendTimer(size_t index, uint16_t timerId, uint millis) const
{
  queue->start(milliseconds{ millis }, { .index = index, .timerId = timerId });
}
