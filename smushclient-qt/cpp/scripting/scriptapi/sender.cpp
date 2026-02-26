#include "../../casting.h"
#include "../../client.h"
#include "../../timermap.h"
#include "../scriptapi.h"
#include "smushclient_qt/src/ffi/sender.cxxqt.h"

using std::string_view;
using std::chrono::milliseconds;

// Public static methods

QString
ScriptApi::MakeRegularExpression(string_view pattern) noexcept
{
  return ffi::regex::from_wildcards(pattern);
}

// Public methods

ApiCode
ScriptApi::AddAlias(size_t plugin,
                    string_view name,
                    string_view pattern,
                    string_view text,
                    AliasFlags flags,
                    string_view scriptName) const noexcept
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

  if (!flags.testFlag(AliasFlag::Replace)) {
    return client.addAlias(plugin, alias);
  }

  client.replaceAlias(plugin, alias);
  return ApiCode::OK;
}

ApiCode
ScriptApi::AddTimer(size_t plugin,
                    string_view name,
                    int hour,
                    int minute,
                    double second,
                    string_view text,
                    TimerFlags flags,
                    string_view scriptName) const noexcept
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
                      TriggerFlags flags,
                      const QColor& color,
                      string_view sound,
                      string_view script,
                      SendTarget target,
                      int sequence) const noexcept
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

  if (!flags.testFlag(TriggerFlag::Replace)) {
    return client.addTrigger(plugin, trigger);
  }

  client.replaceTrigger(plugin, trigger);
  return ApiCode::OK;
}

ApiCode
ScriptApi::DeleteAlias(size_t plugin, string_view name) const noexcept
{
  return client.removeSender(SenderKind::Alias, plugin, name);
}

size_t
ScriptApi::DeleteAliasGroup(size_t plugin, string_view group) const noexcept
{
  return client.removeSenderGroup(SenderKind::Alias, plugin, group);
}

size_t
ScriptApi::DeleteTemporaryAliases() const noexcept
{
  return client.removeTemporarySenders(SenderKind::Alias);
}

size_t
ScriptApi::DeleteTemporaryTimers() const noexcept
{
  return client.removeTemporarySenders(SenderKind::Timer);
}

size_t
ScriptApi::DeleteTemporaryTriggers() const noexcept
{
  return client.removeTemporarySenders(SenderKind::Trigger);
}

ApiCode
ScriptApi::DeleteTimer(size_t plugin, string_view name) const noexcept
{
  return client.removeSender(SenderKind::Timer, plugin, name);
}

size_t
ScriptApi::DeleteTimerGroup(size_t plugin, string_view group) const noexcept
{
  return client.removeSenderGroup(SenderKind::Timer, plugin, group);
}

ApiCode
ScriptApi::DeleteTrigger(size_t plugin, string_view name) const noexcept
{
  return client.removeSender(SenderKind::Trigger, plugin, name);
}

size_t
ScriptApi::DeleteTriggerGroup(size_t plugin, string_view group) const noexcept
{
  return client.removeSenderGroup(SenderKind::Trigger, plugin, group);
}

ApiCode
ScriptApi::DoAfter(size_t plugin,
                   double seconds,
                   const QString& text,
                   SendTarget target)
{
  if (seconds < 0.1 || seconds > 86399) {
    return ApiCode::TimeInvalid;
  }
  const milliseconds duration =
    milliseconds{ clamped_cast<int64_t>(seconds * 1000.0) };
  sendQueue->start(duration,
                   { .plugin = plugin,
                     .sendTo = target,
                     .text = text,
                     .destination = QString() });
  return ApiCode::OK;
}

ApiCode
ScriptApi::EnableAlias(size_t plugin,
                       string_view label,
                       bool enabled) const noexcept
{
  return client.setSenderEnabled(SenderKind::Alias, plugin, label, enabled);
}

size_t
ScriptApi::EnableAliasGroup(size_t plugin,
                            string_view group,
                            bool enabled) const noexcept
{
  return client.setSenderGroupEnabled(
    SenderKind::Alias, plugin, group, enabled);
}

ApiCode
ScriptApi::EnableTimer(size_t plugin,
                       string_view label,
                       bool enabled) const noexcept
{
  return client.setSenderEnabled(SenderKind::Timer, plugin, label, enabled);
}

size_t
ScriptApi::EnableTimerGroup(size_t plugin,
                            string_view group,
                            bool enabled) const noexcept
{
  return client.setSenderGroupEnabled(
    SenderKind::Timer, plugin, group, enabled);
}

ApiCode
ScriptApi::EnableTrigger(size_t plugin,
                         string_view label,
                         bool enabled) const noexcept
{
  return client.setSenderEnabled(SenderKind::Trigger, plugin, label, enabled);
}

size_t
ScriptApi::EnableTriggerGroup(size_t plugin,
                              string_view group,
                              bool enabled) const noexcept
{
  return client.setSenderGroupEnabled(
    SenderKind::Trigger, plugin, group, enabled);
}

QString
ScriptApi::ExportXML(size_t plugin,
                     ExportKind kind,
                     std::string_view name) const noexcept
{
  try {
    return client.tryExportXml(kind, plugin, name);
  } catch (const rust::Error& error) {
    return QString::fromUtf8(error.what());
  }
}

QVariant
ScriptApi::GetAliasOption(size_t plugin,
                          string_view label,
                          string_view option) const noexcept
{
  return client.getSenderOption(SenderKind::Alias, plugin, label, option);
}

QVariant
ScriptApi::GetTimerOption(size_t plugin,
                          string_view label,
                          string_view option) const noexcept
{
  return client.getSenderOption(SenderKind::Timer, plugin, label, option);
}

QVariant
ScriptApi::GetTriggerOption(size_t plugin,
                            string_view label,
                            string_view option) const noexcept
{
  return client.getSenderOption(SenderKind::Trigger, plugin, label, option);
}

ApiCode
ScriptApi::IsAlias(size_t plugin, string_view label) const noexcept
{
  return client.isSender(SenderKind::Alias, plugin, label)
           ? ApiCode::OK
           : ApiCode::AliasNotFound;
}

ApiCode
ScriptApi::IsTimer(size_t plugin, string_view label) const noexcept
{
  return client.isSender(SenderKind::Timer, plugin, label)
           ? ApiCode::OK
           : ApiCode::TimerNotFound;
}

ApiCode
ScriptApi::IsTrigger(size_t plugin, string_view label) const noexcept
{
  return client.isSender(SenderKind::Trigger, plugin, label)
           ? ApiCode::OK
           : ApiCode::TriggerNotFound;
}

ApiCode
ScriptApi::SetAliasOption(size_t plugin,
                          string_view label,
                          string_view option,
                          string_view value) const noexcept
{
  return client.setSenderOption(
    SenderKind::Alias, plugin, label, option, value);
}

ApiCode
ScriptApi::SetTimerOption(size_t plugin,
                          string_view label,
                          string_view option,
                          string_view value) const noexcept
{
  return client.setSenderOption(
    SenderKind::Timer, plugin, label, option, value);
}

ApiCode
ScriptApi::SetTriggerOption(size_t plugin,
                            string_view label,
                            string_view option,
                            string_view value) const noexcept
{
  return client.setSenderOption(
    SenderKind::Trigger, plugin, label, option, value);
}

void
ScriptApi::StopEvaluatingTriggers() const noexcept
{
  client.stopSenders(SenderKind::Trigger);
}
