#include "../../casting.h"
#include "../../ui/worldtab.h"
#include "../scriptapi.h"
#include "smushclient_qt/src/ffi/util.cxx.h"

using std::string_view;

// Public static methods

QStringList
ScriptApi::GetAlphaOptionList() noexcept
{
  return ffi::util::get_alpha_option_list();
}

QVariant
ScriptApi::GetDefaultValue(string_view name)
{
  return ffi::util::default_variant_option(name);
}

QStringList
ScriptApi::GetOptionList() noexcept
{
  return ffi::util::get_option_list();
}

// Public methods

QVariant
ScriptApi::GetAliasOption(size_t plugin,
                          string_view label,
                          string_view option) const noexcept
{
  return client.getSenderOption(SenderKind::Alias, plugin, label, option);
}

string_view
ScriptApi::GetAlphaOption(size_t plugin, string_view name) const noexcept
{
  return client.worldAlphaOption(plugin, name);
}

QVariant
ScriptApi::GetCurrentValue(size_t pluginIndex,
                           string_view option) const noexcept
{
  return client.worldVariantOption(pluginIndex, option);
}

int64_t
ScriptApi::GetOption(size_t plugin, string_view name) const noexcept
{
  return client.worldOption(plugin, name);
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
ScriptApi::SetAlphaOption(size_t plugin, string_view name, string_view value)
{
  return tab.setWorldAlphaOption(plugin, name, value);
}

ApiCode
ScriptApi::SetOption(size_t plugin, string_view name, int64_t value)
{
  const ApiCode code = tab.setWorldOption(plugin, name, value);
  if (code != ApiCode::OK) {
    return code;
  }

  cursor->setOption(name, value);

  if (name == "display_my_input") {
    echoInput = value == 1;
  } else if (name == "naws") {
    doNaws = value == 1;
  } else if (name == "enable_scripts" && worldScriptIndex != noSuchPlugin) {
    setPluginEnabled(worldScriptIndex, value == 1);
  } else if (name == "speed_walk_delay") {
    commandQueueTimer->setInterval(clamped_cast<int>(value));
    if (value == 0) {
      flushCommandQueue();
    }
  }

  return code;
}
