---@meta

---Broadcasts a message to all installed plugins.
---
---All installed plugins are checked for a function OnPluginBroadcast. If present, it is called with 4 arguments:
---
---1. *code* (the message number).
---2. The ID of the calling plugin, or an empty string if `BroadcastPlugin` was called from the main script file.
---3. The name of the calling plugin, or an empty string if `BroadcastPlugin` was called from the main script file.
---4. *text* (the text argument).
---@param code integer Arbitrary message code.
---@param text string Message text.
---@return integer plugins The number of plugins that had an OnPluginBroadcast handler.
---
---@see CallPlugin
---@see GetPluginList
function BroadcastPlugin(code, text) end

---Calls a nominated routine in a nominated plugin, supplying a string argument.
---
---The intention here is to allow plugins to interact with each other, to a certain extent.
---
---For instance, you might write a plugin that logs text to a log file (or miniwindow), and want to share it between other plugins. Hence you might log a message like this:
---
---```lua
---CallPlugin("80cc18937a2aca27079567f0", "LogIt", "Data to be logged")
---```
---
---The above example would locate plugin with the ID 80cc18937a2aca27079567f0 (if installed) and then call the routine "LogIt" in that plugin, with the argument "Data to be logged". In this example, the LogIt routine would look like this:
---
---```lua
------@param sText string
---function LogIt(sText)
---    WriteLog(sText)
---end
---```
---
---You should exercise caution when using this technique. It will be annoying for plugin users if plugins become overly dependent on each other, particularly if they cannot find the one that is required.
---
---Also, be careful you do not set up circular dependencies (eg. A needs B, and B needs A).
---
---You can use [`PluginSupports`](lua://PluginSupports) to see if a particular routine is implemented in a plugin. For example:
---
---```lua
---if PluginSupports("80cc18937a2aca27079567f0", "LogIt") ~= error_code.eOK then
---    Note("Required 'LogIt' routine is not available")
---end
---```
---
---If you want to send a message to all installed plugins you might consider using [`BroadcastPlugin`](lua://BroadcastPlugin) instead.
---@param pluginID string ID of the target plugin.
---@param routine string Name of the target routine.
---@param ... nil|boolean|lightuserdata|number|string
---@return error_code code #
---`error_code.eNoSuchPlugin`: Plugin not installed.\
---`error_code.ePluginDisabled`: Plugin is disabled.\
---`error_code.eNoSuchRoutine`: Specified routine cannot be found in that plugin.\
---`error_code.eErrorCallingPluginRoutine`: Error when calling function (runtime error) OR - the function returned an unsupported data type.\
---`error_code.eBadParameter`: An argument was an invalid type (Lua only).\
---`error_code.eOK`: Called OK.
---@return nil|boolean|lightuserdata|number|string ... On success, these are the values returned by the function. If an error occurs, these are a string indicating the reason for the error, and an optional third return value which is the error message generated at runtime if the result code is `error_code.eErrorCallingPluginRoutine`.
---
---@see BroadcastPlugin
---@see GetPluginList
function CallPlugin(pluginID, routine, ...) end

---Enables or disables a plugin. An enabled plugin is "active", otherwise its triggers, timers and aliases are ignored.
---
---You can use [`GetPluginInfo(17)`](lua://GetPluginInfo) to see if the plugin is currently enabled.
---
---If the plugin's "enabled" state changes (ie. from enabled to disabled, or disabled to enabled) then the appropriate routine in the plugin is called, if found: OnPluginEnable or OnPluginDisable.
---@param pluginID string Plugin ID.
---@param enable? boolean Default: true.
---@return error_code code #
---`error_code.eNoSuchPlugin`: That plugin ID is not installed\
---`error_code.eOK`: enabled/disabled OK
function EnablePlugin(pluginID, enable) end

---Returns the unique ID (identifier) of the currently-executing plugin. This will let the plugin find things out about itself (using [`GetPluginInfo`](lua://GetPluginInfo)).
---@return string pluginID Unique ID, or an empty string if no plugin is executing.
---
---@see GetPluginName
function GetPluginID() end

---Lists the unique IDs of all the plugins currently installed.
---@return string[] pluginIDs Array of unique IDs.
function GetPluginList() end

---Gets the name of the currently-executing plugin.
---@return string pluginName Name, or an empty string if no plugin is executing.
---
---@see GetPluginID
function GetPluginName() end

---Use `IsPluginInstalled` to find if a particular plugin (identified by its unique Plugin ID) has been installed into this session.
---@param pluginID string
---@return boolean installed `true` if the plugin has been installed.
function IsPluginInstalled(pluginID) end

---Checks to see if a specified plugin exists, and supports a specified routine.
---@param pluginID string
---@param routine string
---@return error_code code #
---`error_code.eNoSuchPlugin`: Plugin not installed.\
---`error_code.eNoSuchRoutine`: Specified routine cannot be found in that plugin.\
---`error_code.eOK`: Called OK.
function PluginSupports(pluginID, routine) end
