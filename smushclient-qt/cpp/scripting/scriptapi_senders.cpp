#include "../ui/worldtab.h"
#include "errors.h"
#include "scriptapi.h"
#include "smushclient_qt/src/ffi/sender.cxxqt.h"
#include "sqlite3.h"
#include <QtCore/QFile>
#include <QtGui/QGradient>
#include <QtGui/QGuiApplication>
#include <QtWidgets/QStatusBar>

using std::nullopt;
using std::optional;
using std::string;
using std::string_view;
using std::chrono::hours;
using std::chrono::milliseconds;
using std::chrono::minutes;
using std::chrono::seconds;

// Public methods

ApiCode ScriptApi::AddAlias(size_t plugin, const QString &name,
                            const QString &pattern, const QString &text,
                            QFlags<AliasFlag> flags,
                            const QString &scriptName) const {
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

  try {
    if (!flags.testFlag(AliasFlag::Replace))
      return convertAliasCode(client()->addAlias(plugin, alias));

    client()->replaceAlias(plugin, alias);
    return ApiCode::OK;
  } catch (rust::Error &) {
    return ApiCode::BadRegularExpression;
  }
}

ApiCode ScriptApi::AddTimer(size_t plugin, const QString &name, int hour,
                            int minute, double second, const QString &text,
                            QFlags<TimerFlag> flags,
                            const QString &scriptName) const {
  if (flags.testFlag(TimerFlag::AtTime))
    return ApiCode::OK;

  if (hour < 0 || minute < 0 || second < 0 || hour >= 24 || minute >= 60 ||
      second >= 60)
    return ApiCode::TimeInvalid;

  const SendTarget target =
      flags.testFlag(TimerFlag::TimerSpeedWalk) ? SendTarget::Speedwalk
      : flags.testFlag(TimerFlag::TimerNote)    ? SendTarget::Output
                                                : SendTarget::World;

  Timer timer;
  timer.setActiveClosed(flags.testFlag(TimerFlag::ActiveWhenClosed));
  timer.setEnabled(flags.testFlag(TimerFlag::Enabled));
  timer.setEveryHour(hour);
  timer.setEveryMillisecond((int)(second / 1000));
  timer.setEveryMinute(minute);
  timer.setEverySecond((int)second);
  timer.setLabel(name);
  timer.setOccurrence(Occurrence::Interval);
  timer.setOneShot(flags.testFlag(TimerFlag::OneShot));
  timer.setScript(scriptName);
  timer.setSendTo(target);
  timer.setTemporary(flags.testFlag(TimerFlag::Temporary));
  timer.setText(text);

  if (!flags.testFlag(TimerFlag::Replace))
    return convertTimerCode(client()->addTimer(plugin, timer, *timekeeper));

  client()->replaceTimer(plugin, timer, *timekeeper);
  return ApiCode::OK;
}

ApiCode ScriptApi::AddTrigger(size_t plugin, const QString &name,
                              const QString &pattern, const QString &text,
                              QFlags<TriggerFlag> flags, const QColor &color,
                              const QString &sound, const QString &script,
                              SendTarget target, int sequence) const {
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
  if (color.isValid()) {
    trigger.setChangeForeground(true);
    trigger.setForegroundColor(color);
  }

  try {
    if (!flags.testFlag(TriggerFlag::Replace))
      return convertTriggerCode(client()->addTrigger(plugin, trigger));

    client()->replaceTrigger(plugin, trigger);
    return ApiCode::OK;
  } catch (rust::Error &) {
    return ApiCode::BadRegularExpression;
  }
}

ApiCode ScriptApi::DeleteAlias(size_t index, const QString &name) const {
  return convertAliasCode(client()->removeAlias(index, name));
}

size_t ScriptApi::DeleteAliases(size_t index, const QString &group) const {
  return client()->removeAliases(index, group);
}

ApiCode ScriptApi::DeleteTimer(size_t index, const QString &name) const {
  return convertTimerCode(client()->removeTimer(index, name));
}

size_t ScriptApi::DeleteTimers(size_t index, const QString &group) const {
  return client()->removeTimers(index, group);
}

ApiCode ScriptApi::DeleteTrigger(size_t index, const QString &name) const {
  return convertTriggerCode(client()->removeTrigger(index, name));
}

size_t ScriptApi::DeleteTriggers(size_t index, const QString &group) const {
  return client()->removeTriggers(index, group);
}

ApiCode ScriptApi::EnableAlias(size_t plugin, const QString &label,
                               bool enabled) const {
  return convertAliasCode(client()->setAliasEnabled(plugin, label, enabled));
}

ApiCode ScriptApi::EnableAliasGroup(size_t plugin, const QString &group,
                                    bool enabled) const {
  return client()->setAliasesEnabled(plugin, group, enabled)
             ? ApiCode::OK
             : ApiCode::AliasNotFound;
}

ApiCode ScriptApi::EnableTimer(size_t plugin, const QString &label,
                               bool enabled) const {
  return convertTimerCode(client()->setTimerEnabled(plugin, label, enabled));
}

ApiCode ScriptApi::EnableTimerGroup(size_t plugin, const QString &group,
                                    bool enabled) const {
  return client()->setTimersEnabled(plugin, group, enabled)
             ? ApiCode::OK
             : ApiCode::TimerNotFound;
}

ApiCode ScriptApi::EnableTrigger(size_t plugin, const QString &label,
                                 bool enabled) const {
  return convertTriggerCode(
      client()->setTriggerEnabled(plugin, label, enabled));
}

ApiCode ScriptApi::EnableTriggerGroup(size_t plugin, const QString &group,
                                      bool enabled) const {
  return client()->setTriggersEnabled(plugin, group, enabled)
             ? ApiCode::OK
             : ApiCode::TriggerNotFound;
}

ApiCode ScriptApi::IsAlias(size_t plugin, const QString &label) const {
  return client()->isAlias(plugin, label) ? ApiCode::OK
                                          : ApiCode::AliasNotFound;
}

ApiCode ScriptApi::IsTimer(size_t plugin, const QString &label) const {
  return client()->isTimer(plugin, label) ? ApiCode::OK
                                          : ApiCode::TimerNotFound;
}

ApiCode ScriptApi::IsTrigger(size_t plugin, const QString &label) const {
  return client()->isTrigger(plugin, label) ? ApiCode::OK
                                            : ApiCode::TriggerNotFound;
}

ApiCode ScriptApi::SetTriggerGroup(size_t plugin, const QString &label,
                                   const QString &group) const {
  return convertTriggerCode(client()->setTriggerGroup(plugin, label, group));
}

ApiCode ScriptApi::SetTriggerOption(size_t plugin, const QString &label,
                                    TriggerBool option, bool value) const {
  return convertTriggerCode(
      client()->setTriggerBool(plugin, label, option, value));
}
