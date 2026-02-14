#include "../bytes.h"
#include "../casting.h"
#include "../ui/worldtab.h"
#include "scriptapi.h"
#include "smushclient_qt/src/ffi/sender.cxxqt.h"
#include "sqlite3.h"
#include <QtCore/QFile>
#include <QtGui/QGradient>
#include <QtGui/QGuiApplication>
#include <QtWidgets/QStatusBar>

using std::string;
using std::string_view;
using std::chrono::hours;
using std::chrono::milliseconds;
using std::chrono::minutes;
using std::chrono::seconds;

// Public methods

ApiCode
ScriptApi::AddAlias(size_t plugin,
                    string_view name,
                    string_view pattern,
                    string_view text,
                    QFlags<AliasFlag> flags,
                    string_view scriptName) const
{
  if (pattern.empty()) {
    return ApiCode::TriggerCannotBeEmpty;
  }

  const SendTarget target =
    flags.testFlag(AliasFlag::AliasSpeedWalk) ? SendTarget::Speedwalk
    : flags.testFlag(AliasFlag::AliasQueue)   ? SendTarget::WorldDelay
                                              : SendTarget::World;

  Alias alias;
  alias.setSendTo(target);
  alias.setLabel(QString::fromUtf8(name));
  alias.setPattern(QString::fromUtf8(pattern));
  alias.setText(QString::fromUtf8(text));
  alias.setScript(QString::fromUtf8(scriptName));
  alias.setEnabled(flags.testFlag(AliasFlag::Enabled));
  alias.setKeepEvaluating(flags.testFlag(AliasFlag::KeepEvaluating));
  alias.setOmitFromLog(flags.testFlag(AliasFlag::OmitFromLogFile));
  alias.setIsRegex(flags.testFlag(AliasFlag::RegularExpression));
  alias.setExpandVariables(flags.testFlag(AliasFlag::ExpandVariables));
  alias.setMenu(flags.testFlag(AliasFlag::AliasMenu));
  alias.setTemporary(flags.testFlag(AliasFlag::Temporary));

  try {
    if (!flags.testFlag(AliasFlag::Replace)) {
      return client.addAlias(plugin, alias);
    }

    client.replaceAlias(plugin, alias);
    return ApiCode::OK;
  } catch (rust::Error&) {
    return ApiCode::BadRegularExpression;
  }
}

ApiCode
ScriptApi::AddTimer(size_t plugin,
                    string_view name,
                    int hour,
                    int minute,
                    double second,
                    string_view text,
                    QFlags<TimerFlag> flags,
                    string_view scriptName) const
{
  if (flags.testFlag(TimerFlag::AtTime)) {
    return ApiCode::OK;
  }

  if (hour < 0 || minute < 0 || second < 0 || hour >= 24 || minute >= 60 ||
      second >= 60) {
    return ApiCode::TimeInvalid;
  }

  const SendTarget target =
    flags.testFlag(TimerFlag::TimerSpeedWalk) ? SendTarget::Speedwalk
    : flags.testFlag(TimerFlag::TimerNote)    ? SendTarget::Output
                                              : SendTarget::World;

  Timer timer;
  timer.setActiveClosed(flags.testFlag(TimerFlag::ActiveWhenClosed));
  timer.setEnabled(flags.testFlag(TimerFlag::Enabled));
  timer.setEveryHour(hour);
  timer.setEveryMillisecond(clamped_cast<int>(second / 1000));
  timer.setEveryMinute(minute);
  timer.setEverySecond(clamped_cast<int>(second));
  timer.setLabel(QString::fromUtf8(name));
  timer.setOccurrence(Occurrence::Interval);
  timer.setOneShot(flags.testFlag(TimerFlag::OneShot));
  timer.setScript(QString::fromUtf8(scriptName));
  timer.setSendTo(target);
  timer.setTemporary(flags.testFlag(TimerFlag::Temporary));
  timer.setText(QString::fromUtf8(text));

  if (!flags.testFlag(TimerFlag::Replace)) {
    return client.addTimer(plugin, timer, *timekeeper);
  }

  client.replaceTimer(plugin, timer, *timekeeper);
  return ApiCode::OK;
}

ApiCode
ScriptApi::AddTrigger(size_t plugin,
                      string_view name,
                      string_view pattern,
                      string_view text,
                      QFlags<TriggerFlag> flags,
                      const QColor& color,
                      string_view sound,
                      string_view script,
                      SendTarget target,
                      int sequence) const
{
  if (pattern.empty()) {
    return ApiCode::TriggerCannotBeEmpty;
  }

  if (sequence < 0 || sequence > 10000) {
    return ApiCode::TriggerSequenceOutOfRange;
  }

  Trigger trigger;
  trigger.setEnabled(flags.testFlag(TriggerFlag::Enabled));
  trigger.setExpandVariables(flags.testFlag(TriggerFlag::ExpandVariables));
  trigger.setIgnoreCase(flags.testFlag(TriggerFlag::IgnoreCase));
  trigger.setIsRegex(flags.testFlag(TriggerFlag::RegularExpression));
  trigger.setKeepEvaluating(flags.testFlag(TriggerFlag::KeepEvaluating));
  trigger.setLabel(QString::fromUtf8(name));
  trigger.setLowercaseWildcard(flags.testFlag(TriggerFlag::LowercaseWildcard));
  trigger.setOmitFromLog(flags.testFlag(TriggerFlag::OmitFromLog));
  trigger.setOmitFromOutput(flags.testFlag(TriggerFlag::OmitFromOutput));
  trigger.setPattern(QString::fromUtf8(pattern));
  trigger.setScript(QString::fromUtf8(script));
  trigger.setSendTo(target);
  trigger.setSequence(sequence);
  trigger.setSound(QString::fromUtf8(sound));
  trigger.setText(QString::fromUtf8(text));
  if (color.isValid()) {
    trigger.setChangeForeground(true);
    trigger.setForegroundColor(color);
  }

  try {
    if (!flags.testFlag(TriggerFlag::Replace)) {
      return client.addTrigger(plugin, trigger);
    }

    client.replaceTrigger(plugin, trigger);
    return ApiCode::OK;
  } catch (rust::Error&) {
    return ApiCode::BadRegularExpression;
  }
}

ApiCode
ScriptApi::DeleteAlias(size_t plugin, string_view name) const
{
  return client.removeAlias(plugin, bytes::slice(name));
}

size_t
ScriptApi::DeleteAliasGroup(size_t plugin, string_view group) const
{
  return client.removeAliasGroup(plugin, bytes::slice(group));
}

ApiCode
ScriptApi::DeleteTimer(size_t plugin, string_view name) const
{
  return client.removeTimer(plugin, bytes::slice(name));
}

size_t
ScriptApi::DeleteTimerGroup(size_t plugin, string_view group) const
{
  return client.removeTimerGroup(plugin, bytes::slice(group));
}

ApiCode
ScriptApi::DeleteTrigger(size_t plugin, string_view name) const
{
  return client.removeTrigger(plugin, bytes::slice(name));
}

size_t
ScriptApi::DeleteTriggerGroup(size_t plugin, string_view group) const
{
  return client.removeTriggerGroup(plugin, bytes::slice(group));
}

ApiCode
ScriptApi::EnableAlias(size_t plugin, string_view label, bool enabled) const
{
  return client.setAliasEnabled(plugin, bytes::slice(label), enabled);
}

ApiCode
ScriptApi::EnableAliasGroup(size_t plugin,
                            string_view group,
                            bool enabled) const
{
  return client.setAliasesEnabled(plugin, bytes::slice(group), enabled)
           ? ApiCode::OK
           : ApiCode::AliasNotFound;
}

ApiCode
ScriptApi::EnableTimer(size_t plugin, string_view label, bool enabled) const
{
  return client.setTimerEnabled(plugin, bytes::slice(label), enabled);
}

ApiCode
ScriptApi::EnableTimerGroup(size_t plugin,
                            string_view group,
                            bool enabled) const
{
  return client.setTimersEnabled(plugin, bytes::slice(group), enabled)
           ? ApiCode::OK
           : ApiCode::TimerNotFound;
}

ApiCode
ScriptApi::EnableTrigger(size_t plugin, string_view label, bool enabled) const
{
  return client.setTriggerEnabled(plugin, bytes::slice(label), enabled);
}

ApiCode
ScriptApi::EnableTriggerGroup(size_t plugin,
                              string_view group,
                              bool enabled) const
{
  return client.setTriggersEnabled(plugin, bytes::slice(group), enabled)
           ? ApiCode::OK
           : ApiCode::TriggerNotFound;
}

QVariant
ScriptApi::GetAliasOption(size_t plugin,
                          string_view label,
                          string_view option) const
{
  return client.getAliasOption(
    plugin, bytes::slice(label), bytes::slice(option));
}

QVariant
ScriptApi::GetTimerOption(size_t plugin,
                          string_view label,
                          string_view option) const
{
  return client.getTimerOption(
    plugin, bytes::slice(label), bytes::slice(option));
}

QVariant
ScriptApi::GetTriggerOption(size_t plugin,
                            string_view label,
                            string_view option) const
{
  return client.getTriggerOption(
    plugin, bytes::slice(label), bytes::slice(option));
}

ApiCode
ScriptApi::IsAlias(size_t plugin, string_view label) const
{
  return client.isAlias(plugin, bytes::slice(label)) ? ApiCode::OK
                                                     : ApiCode::AliasNotFound;
}

ApiCode
ScriptApi::IsTimer(size_t plugin, string_view label) const
{
  return client.isTimer(plugin, bytes::slice(label)) ? ApiCode::OK
                                                     : ApiCode::TimerNotFound;
}

ApiCode
ScriptApi::IsTrigger(size_t plugin, string_view label) const
{
  return client.isTrigger(plugin, bytes::slice(label))
           ? ApiCode::OK
           : ApiCode::TriggerNotFound;
}

ApiCode
ScriptApi::SetAliasOption(size_t plugin,
                          string_view label,
                          string_view option,
                          string_view value) const
{
  return client.setAliasOption(
    plugin, bytes::slice(label), bytes::slice(option), bytes::slice(value));
}

ApiCode
ScriptApi::SetTimerOption(size_t plugin,
                          string_view label,
                          string_view option,
                          string_view value) const
{
  return client.setTimerOption(
    plugin, bytes::slice(label), bytes::slice(option), bytes::slice(value));
}

ApiCode
ScriptApi::SetTriggerOption(size_t plugin,
                            string_view label,
                            string_view option,
                            string_view value) const
{
  return client.setTriggerOption(
    plugin, bytes::slice(label), bytes::slice(option), bytes::slice(value));
}
