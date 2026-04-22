#include "../../client.h"
#include "../../ui/worldtab.h"
#include "../callback/plugincallback.h"
#include "../scriptapi.h"
#include "smushclient_qt/src/ffi/util.cxx.h"

using std::string_view;

// Public static methods

string_view
ScriptApi::GetXMLEntity(string_view name) noexcept
{
  return ffi::util::get_global_entity(name);
}

// Public methods

ApiCode
ScriptApi::DeleteVariable(size_t plugin, string_view key) const noexcept
{
  return client.unsetVariable(plugin, key) ? ApiCode::OK
                                           : ApiCode::VariableNotFound;
}

string_view
ScriptApi::GetEntity(string_view name) const noexcept
{
  return client.getMxpEntity(name);
}

string_view
ScriptApi::GetVariable(size_t index, string_view key) const noexcept
{

  return client.getVariable(index, key);
}

string_view
ScriptApi::GetVariable(string_view pluginID, string_view key) const noexcept
{
  const size_t index = findPluginIndex(pluginID);
  if (index == noSuchPlugin) {
    return {};
  }
  return GetVariable(index, key);
}

rust::Vec<VariableEntry>
ScriptApi::GetVariableList(size_t pluginIndex) const noexcept
{
  return client.variableEntries(pluginIndex);
}

rust::Vec<VariableEntry>
ScriptApi::GetVariableList(string_view pluginID) const
{
  const size_t index = findPluginIndex(pluginID);
  if (index == noSuchPlugin) {
    return {};
  }
  return client.variableEntries(index);
}

ApiCode
ScriptApi::SaveState(size_t plugin)
{
  OnPluginSaveState onSaveState;
  sendCallback(onSaveState, plugin);
  try {
    client.trySaveState(plugin, tab.variablesPath());
  } catch (const rust::Error&) {
    return ApiCode::PluginCouldNotSaveState;
  }
  return ApiCode::OK;
}

bool
ScriptApi::SetEntity(string_view name, string_view value) const noexcept
{
  return client.setMxpEntity(name, value);
}

bool
ScriptApi::SetVariable(size_t index,
                       string_view key,
                       string_view value) const noexcept
{
  return client.setVariable(index, key, value);
}
