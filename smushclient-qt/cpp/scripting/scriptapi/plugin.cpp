#include "../../client.h"
#include "../callback/plugincallback.h"
#include "../scriptapi.h"

using std::string;
using std::string_view;

// Public methods

int64_t
ScriptApi::BroadcastPlugin(size_t index,
                           int64_t message,
                           string_view text) const
{
  const Plugin& callingPlugin = plugins[index];
  int64_t calledPlugins = 0;
  OnPluginBroadcast onBroadcast(
    message, callingPlugin.id(), callingPlugin.name(), text);
  for (const Plugin& plugin : plugins) {
    if (&plugin != &callingPlugin && plugin.runCallbackThreaded(onBroadcast)) {
      ++calledPlugins;
    }
  }
  return calledPlugins;
}

ApiCode
ScriptApi::EnablePlugin(string_view pluginID, bool enabled)
{
  const size_t index = findPluginIndex(pluginID);
  if (index == noSuchPlugin) {
    return ApiCode::NoSuchPlugin;
  }
  setPluginEnabled(index, enabled);
  client.setPluginEnabled(index, enabled);
  return ApiCode::OK;
}

const string&
ScriptApi::GetPluginID(size_t index) const
{
  return plugins.at(index).id();
}

const string&
ScriptApi::GetPluginName(size_t index) const
{
  return plugins.at(index).name();
}

ApiCode
ScriptApi::PluginSupports(string_view pluginID, PluginCallbackKey routine) const
{
  const size_t index = findPluginIndex(pluginID);
  if (index == noSuchPlugin) [[unlikely]] {
    return ApiCode::NoSuchPlugin;
  }
  return plugins[index].hasFunction(routine) ? ApiCode::OK
                                             : ApiCode::NoSuchRoutine;
}
