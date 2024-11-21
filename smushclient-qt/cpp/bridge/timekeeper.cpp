#include "timekeeper.h"
#include "../scripting/scriptapi.h"
#include "../scripting/plugincallback.h"
#include "cxx-qt-gen/ffi.cxxqt.h"
extern "C"
{
#include "lua.h"
}

using std::chrono::milliseconds;

class TimerCallback : public PluginCallback
{
public:
  TimerCallback(const rust::String &callback, const rust::String &label)
      : callback(callback.data()),
        label(label) {}

  inline constexpr const char *name() const noexcept override { return callback; }
  inline constexpr ActionSource source() const noexcept override { return ActionSource::TimerFired; }

  int pushArguments(lua_State *L) const override
  {
    lua_pushlstring(L, label.data(), label.size());
    return 1;
  }

private:
  const char *callback;
  const rust::String &label;
};

Timekeeper::Timekeeper(ScriptApi *parent)
    : QObject(parent),
      closed(true),
      pollTimerId(-1),
      queue() {}

void Timekeeper::beginPolling(milliseconds interval, Qt::TimerType timerType)
{
  pollTimerId = startTimer(interval, timerType);
}

void Timekeeper::sendTimer(const SendTimer &timer) const
{
  if (closed && !timer.activeClosed)
    return;

  ScriptApi *api = getApi();
  const ActionSource oldSource = api->setSource(ActionSource::TimerFired);
  api->sendTo(timer.plugin, timer.target, timer.text);
  api->setSource(oldSource);

  if (timer.script.empty())
    return;

  TimerCallback callback(timer.script, timer.label);
  api->sendCallback(callback, timer.plugin);
}

void Timekeeper::startSendTimer(size_t id, uint ms)
{
  queue[startTimer(milliseconds{ms})] = id;
}

// protected overrides

void Timekeeper::timerEvent(QTimerEvent *event)
{
  const int id = event->timerId();
  if (id == pollTimerId)
  {
    getApi()->client()->pollTimers(*this);
    return;
  }
  auto search = queue.find(id);
  if (search == queue.end()) [[unlikely]]
  {
    killTimer(id);
    return;
  }
  if (getApi()->client()->finishTimer(search->second, *this))
    killTimer(id);
}

// Private methods

inline ScriptApi *Timekeeper::getApi() const { return qobject_cast<ScriptApi *>(parent()); }
