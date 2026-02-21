#include "../../client.h"
#include "../scriptapi.h"

using std::string_view;

// Public methods

ApiCode
ScriptApi::DeleteVariable(size_t plugin, string_view key) const
{
  return client.unsetVariable(plugin, key) ? ApiCode::OK
                                           : ApiCode::VariableNotFound;
}

VariableView
ScriptApi::GetVariable(size_t index, string_view key) const
{

  return client.getVariable(index, key);
}

VariableView
ScriptApi::GetVariable(string_view pluginID, string_view key) const
{
  const size_t index = findPluginIndex(pluginID);
  if (index == noSuchPlugin) {
    return VariableView(nullptr, 0);
  }
  return GetVariable(index, key);
}

bool
ScriptApi::SetVariable(size_t index, string_view key, string_view value) const
{
  return client.setVariable(index, key, value);
}
