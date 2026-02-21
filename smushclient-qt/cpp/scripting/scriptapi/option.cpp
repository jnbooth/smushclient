#include "../../bytes.h"
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

QStringList
ScriptApi::GetOptionList() noexcept
{
  return ffi::util::get_option_list();
}

// Public methods

VariableView
ScriptApi::GetAlphaOption(size_t plugin, string_view name) const
{
  return client.worldAlphaOption(plugin, bytes::slice(name));
}

QVariant
ScriptApi::GetCurrentValue(size_t pluginIndex, string_view option) const
{
  return client.worldVariantOption(pluginIndex, bytes::slice(option));
}

int64_t
ScriptApi::GetOption(size_t plugin, string_view name) const
{
  return client.worldOption(plugin, bytes::slice(name));
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
  }

  return code;
}
