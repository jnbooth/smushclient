#include "scriptapi.h"
#include <QtCore/QFile>
#include <QtGui/QClipboard>
#include <QtGui/QGradient>
#include <QtGui/QGuiApplication>
#include <QtWidgets/QColorDialog>
#include <QtWidgets/QStatusBar>
#include "sqlite3.h"
#include "miniwindow.h"
#include "worldproperties.h"
#include "../../spans.h"
#include "../ui/worldtab.h"
#include "../ui/ui_worldtab.h"

using std::nullopt;
using std::optional;
using std::string;
using std::string_view;
using std::chrono::hours;
using std::chrono::milliseconds;
using std::chrono::minutes;
using std::chrono::seconds;

// Public methods

ApiCode ScriptApi::AddAlias(
    size_t plugin,
    const QString &name,
    const QString &pattern,
    const QString &text,
    QFlags<AliasFlag> flags,
    const QString &scriptName) const
{
  if (pattern.isEmpty())
    return ApiCode::TriggerCannotBeEmpty;

  const SendTarget target =
      flags.testFlag(AliasFlag::AliasSpeedWalk) ? SendTarget::Speedwalk
      : flags.testFlag(AliasFlag::AliasQueue)   ? SendTarget::WorldDelay
                                                : SendTarget::World;

  Alias alias;
  alias.setSendTo(target);
  alias.setLabel(name);
  alias.setPattern(pattern);
  alias.setText(text);
  alias.setScript(scriptName);
  alias.setEnabled(flags.testFlag(AliasFlag::Enabled));
  alias.setKeepEvaluating(flags.testFlag(AliasFlag::KeepEvaluating));
  alias.setOmitFromLog(flags.testFlag(AliasFlag::OmitFromLogFile));
  alias.setIsRegex(flags.testFlag(AliasFlag::RegularExpression));
  alias.setExpandVariables(flags.testFlag(AliasFlag::ExpandVariables));
  alias.setMenu(flags.testFlag(AliasFlag::AliasMenu));
  alias.setTemporary(flags.testFlag(AliasFlag::Temporary));

  try
  {
    const ssize_t index = flags.testFlag(AliasFlag::Replace)
                              ? client()->addAlias(plugin, alias)
                              : client()->replaceAlias(plugin, alias);
    return index < 0 ? ApiCode::AliasAlreadyExists : ApiCode::OK;
  }
  catch (rust::Error &)
  {
    return ApiCode::BadRegularExpression;
  }
}

ApiCode ScriptApi::AddTimer(
    size_t plugin,
    const QString &name,
    int hour,
    int minute,
    double second,
    const QString &text,
    QFlags<TimerFlag> flags,
    const QString &scriptName) const
{
  if (flags.testFlag(TimerFlag::AtTime))
    return ApiCode::OK;

  if (hour < 0 || minute < 0 || second < 0 || hour >= 24 || minute >= 60 || second >= 60)
    return ApiCode::TimeInvalid;

  const SendTarget target =
      flags.testFlag(TimerFlag::TimerSpeedWalk) ? SendTarget::Speedwalk
      : flags.testFlag(TimerFlag::TimerNote)    ? SendTarget::Output
                                                : SendTarget::World;

  Timer timer;
  timer.setActiveClosed(flags.testFlag(TimerFlag::ActiveWhenClosed));
  timer.setEnabled(flags.testFlag(TimerFlag::Enabled));
  timer.setEveryHour(hour);
  timer.setEveryMillisecond((int)(second * 1000));
  timer.setEveryMinute(minute);
  timer.setEverySecond((int)second);
  timer.setLabel(name);
  timer.setOccurrence(Occurrence::Interval);
  timer.setOneShot(flags.testFlag(TimerFlag::OneShot));
  timer.setScript(scriptName);
  timer.setSendTo(target);
  timer.setTemporary(flags.testFlag(TimerFlag::Temporary));
  timer.setText(text);

  const ssize_t index = flags.testFlag(TimerFlag::Replace)
                            ? client()->addTimer(plugin, timer, *timekeeper)
                            : client()->replaceTimer(plugin, timer, *timekeeper);

  return index < 0 ? ApiCode::TimerAlreadyExists : ApiCode::OK;
}

ApiCode ScriptApi::AddTrigger(
    size_t plugin,
    const QString &name,
    const QString &pattern,
    const QString &text,
    QFlags<TriggerFlag> flags,
    const QColor &color,
    const QString &sound,
    const QString &script,
    SendTarget target,
    int sequence) const
{
  if (pattern.isEmpty())
    return ApiCode::TriggerCannotBeEmpty;

  if (sequence < 0 || sequence > 10000)
    return ApiCode::TriggerSequenceOutOfRange;

  Trigger trigger;
  trigger.setEnabled(flags.testFlag(TriggerFlag::Enabled));
  trigger.setExpandVariables(flags.testFlag(TriggerFlag::ExpandVariables));
  trigger.setIgnoreCase(flags.testFlag(TriggerFlag::IgnoreCase));
  trigger.setIsRegex(flags.testFlag(TriggerFlag::RegularExpression));
  trigger.setKeepEvaluating(flags.testFlag(TriggerFlag::KeepEvaluating));
  trigger.setLabel(name);
  trigger.setLowercaseWildcard(flags.testFlag(TriggerFlag::LowercaseWildcard));
  trigger.setOmitFromLog(flags.testFlag(TriggerFlag::OmitFromLog));
  trigger.setOmitFromOutput(flags.testFlag(TriggerFlag::OmitFromOutput));
  trigger.setPattern(pattern);
  trigger.setScript(script);
  trigger.setSendTo(target);
  trigger.setSequence(sequence);
  trigger.setSound(sound);
  trigger.setText(text);
  if (color.isValid())
  {
    trigger.setChangeForeground(true);
    trigger.setForegroundColor(color);
  }

  try
  {
    const ssize_t index = flags.testFlag(TriggerFlag::Replace)
                              ? client()->addTrigger(plugin, trigger)
                              : client()->replaceTrigger(plugin, trigger);
    return index < 0 ? ApiCode::TriggerAlreadyExists : ApiCode::OK;
  }
  catch (rust::Error &)
  {
    return ApiCode::BadRegularExpression;
  }
}

ApiCode ScriptApi::DeleteAlias(size_t index, const QString &name) const
{
  return client()->removeAlias(index, name) ? ApiCode::OK : ApiCode::AliasNotFound;
}

size_t ScriptApi::DeleteAliases(size_t index, const QString &group) const
{
  return client()->removeAliases(index, group);
}

ApiCode ScriptApi::DeleteTimer(size_t index, const QString &name) const
{
  return client()->removeTimer(index, name) ? ApiCode::OK : ApiCode::TimerNotFound;
}

size_t ScriptApi::DeleteTimers(size_t index, const QString &group) const
{
  return client()->removeTimers(index, group);
}

ApiCode ScriptApi::DeleteTrigger(size_t index, const QString &name) const
{
  return client()->removeTrigger(index, name) ? ApiCode::OK : ApiCode::TriggerNotFound;
}

size_t ScriptApi::DeleteTriggers(size_t index, const QString &group) const
{
  return client()->removeTriggers(index, group);
}

ApiCode ScriptApi::EnableAlias(const QString &label, bool enabled) const
{
  return client()->setAliasEnabled(label, enabled) ? ApiCode::OK : ApiCode::AliasNotFound;
}

ApiCode ScriptApi::EnableAliasGroup(const QString &group, bool enabled) const
{
  return client()->setAliasesEnabled(group, enabled) ? ApiCode::OK : ApiCode::AliasNotFound;
}

ApiCode ScriptApi::EnableTimer(const QString &label, bool enabled) const
{
  return client()->setTimerEnabled(label, enabled) ? ApiCode::OK : ApiCode::AliasNotFound;
}

ApiCode ScriptApi::EnableTimerGroup(const QString &group, bool enabled) const
{
  return client()->setTimersEnabled(group, enabled) ? ApiCode::OK : ApiCode::AliasNotFound;
}

ApiCode ScriptApi::EnableTrigger(const QString &label, bool enabled) const
{
  return client()->setTriggerEnabled(label, enabled) ? ApiCode::OK : ApiCode::AliasNotFound;
}

ApiCode ScriptApi::EnableTriggerGroup(const QString &group, bool enabled) const
{
  return client()->setTriggersEnabled(group, enabled) ? ApiCode::OK : ApiCode::AliasNotFound;
}

ApiCode ScriptApi::IsAlias(const QString &label) const
{
  return client()->isAlias(label) ? ApiCode::OK : ApiCode::AliasNotFound;
}

ApiCode ScriptApi::IsTimer(const QString &label) const
{
  return client()->isTimer(label) ? ApiCode::OK : ApiCode::TimerNotFound;
}

ApiCode ScriptApi::IsTrigger(const QString &label) const
{
  return client()->isTrigger(label) ? ApiCode::OK : ApiCode::TriggerNotFound;
}
