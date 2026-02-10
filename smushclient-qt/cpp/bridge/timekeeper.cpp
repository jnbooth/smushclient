#include "timekeeper.h"
#include "../client.h"
#include "../scripting/callback/plugincallback.h"
#include "../scripting/scriptapi.h"
#include "../timer_map.h"
#include "smushclient_qt/src/ffi/timekeeper.cxxqt.h"
extern "C"
{
#include "lua.h"
}

using std::chrono::milliseconds;
using std::chrono::seconds;

class TimerCallback : public DynamicPluginCallback
{
public:
  TimerCallback(const rust::String& callback, const rust::String& label)
    : DynamicPluginCallback(callback)
    , label(label)
  {
  }

  constexpr ActionSource source() const noexcept override
  {
    return ActionSource::TimerFired;
  }

  int pushArguments(lua_State* L) const override
  {
    lua_pushlstring(L, label.data(), label.size());
    return 1;
  }

private:
  const rust::String& label;
};

Timekeeper::Timekeeper(const SmushClient& client, ScriptApi& parent)
  : QObject(&parent)
  , api(parent)
  , client(client)
  , pollTimer(new QTimer(this))
  , queue(new TimerMap<Timekeeper::Item>(this, &Timekeeper::finishTimer))
{
  connect(pollTimer, &QTimer::timeout, this, &Timekeeper::pollTimers);
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
Timekeeper::sendTimer(const SendTimer& timer) const
{
  if (closed && !timer.activeClosed) {
    return;
  }

  const ActionSource oldSource = api.setSource(ActionSource::TimerFired);
  api.handleSendRequest(timer.request);
  api.setSource(oldSource);

  if (timer.script.empty()) {
    return;
  }

  TimerCallback callback(timer.script, timer.label);
  api.sendCallback(callback, timer.request.plugin);
}

void
Timekeeper::startSendTimer(size_t index, uint16_t timerId, uint millis) const
{
  queue->start(milliseconds{ millis }, { .index = index, .timerId = timerId });
}

// Private slots

bool
Timekeeper::finishTimer(const Timekeeper::Item& item)
{
  return client.finishTimer(item.index, *this);
}

void
Timekeeper::pollTimers()
{
  client.pollTimers(*this);
}
