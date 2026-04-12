---@meta

---Deletes the named variable.
---@param variableName string
---@return error_code code #
---`error_code.eInvalidObjectLabel`: The alias name is not valid.\
---`error_code.eVariableNotFound`: The variable does not exist.\
---`error_code.eOK`: Deleted OK.
---@see GetVariable
---@see GetVariableList
function DeleteVariable(variableName) end

---Returns the value of an entity defined by the MUD server using MXP.
---
---eg.
---
---If the server sent:
---
---```xml
---<!ENTITY foo "bar">
---```
---
---Then if you did this:
---
---```lua
---Note(GetEntity("foo"))
---```
---
---You would see `"bar"` displayed.
---
---To get standard (automatically defined) entities use [`GetXMLEntity`](lua://GetXMLEntity).
---@param name string
---@return string contents The contents of the specified entity, or an empty string if no such entity is found.
---
---@see GetXMLEntity
function GetEntity(name) end

---Gets the contents of the named variable for the nominated plugin. Each plugin has a unique ID - specify that ID to identify the plugin in question.
---
---If you want to find the value of a variable in the current plugin, use [`GetVariable`](lua://GetVariable).
---
---If you are writing a plugin and want to find a "global" variable value, use an empty plugin ID, eg.
---
---```lua
---Note(GetPluginVariable("", "target")))
---```
---@param pluginID string
---@param variableName string
---@return string|nil contents Variable contents, or nil if the specified plugin does not exist or does not have a variable by that name.
---
---@see GetPluginVariableList
---@see GetVariable
function GetPluginVariable(pluginID, variableName) end

---Returns a table of all the variables currently defined for the nominated plugin, keyed by the variable name.
---
---Thus you can directly access variables from the table, like this:
---
----- show value for variable "victim" in plugin ID "982581e59ab42844527eec80" ...
---
---```lua
---print(GetPluginVariableList("982581e59ab42844527eec80").victim)
---```
---
---If you want to find the table of variables in the current plugin, use [`GetVariableList`](lua://GetVariableList).
---
---If you are writing a plugin and want to find the "global" MUSHclient variable list, use an empty plugin ID, eg.
---
---```lua
---local vList = GetPluginVariableList("")
---```
---@param pluginID string
---@return { [string]: string } variables
---
---@see GetPluginVariable
---@see GetVariableList
function GetPluginVariableList(pluginID) end

---Returns the value of a standard XML/MXP (HTML) entity.
---
---To get user (server defined) entities use [`GetEntity`](lua://GetEntity).
---
---Entity names are the standard HTML ones.
---
---You can also use entities in the form: `#nnn` where *nnn* is a decimal number in the range 32 to 255.
---
---You can also use entities in the form: `#xhh` where *hh* is a hexadecimal number in the range 20 to FF.
---
---The entity name should *not* include the leading ampersand or trailing semicolon that you use in XML, MXP or HTML.
---@param name string Entity name, without leading ampersand or trailing semicolon.
---@return string contents The contents of the specified entity, or an empty string if no such entity is found, or the syntax is invalid (eg. #999).
---
---@see GetEntity
function GetXMLEntity(name) end

---Gets the contents of the named variable.
---
---If `GetVariable` is called from within a plugin, the variables for the current plugin are used, not the "global" MUSHclient variables.
---
---If you want to find the value of a variable in another plugin, or the global variables, use [`GetPluginVariable`](lua://GetPluginVariable).
---@param variableName string
---@return string|nil contents Variable contents, or nil if the specified plugin does not exist or does not have a variable by that name.
---
---@see GetPluginVariable
---@see GetVariableList
function GetVariable(variableName) end

---Returns a table of all the variables currently defined for the current plugin or world state, keyed by the variable name.
---
---Thus you can directly access variables from the table, like this:
---
---```lua
----- show value for variable "victim"
---print(GetVariableList().victim)
---```
---@return { [string]: string } variables
---@see GetVariable
---@see GetPluginVariableList
function GetVariableList() end

---This causes the current plugin to save its state (ie. its variables) to the world's variablese file. You might call this in a timer every couple of minutes to ensure that variables are saved in the event of a program or computer crash.
---
---If a plugin has: `save_state="y"` in its plugin configuration (header), then its state is saved automatically as described below.
---
---Plugins automatically save their state when they are closed (ie. when they are uninstalled, or when the world closes), and when the world file is saved. However if the world does not close for days then you may want to arrange for the plugin to save its state more often, particularly if the variables that the plugin is using are valuable.
---
---Plugins are also forced to save their state when the main world file is saved (ie. [`Save`](lua://Save) is called, or you save the world file from the File menu).
---
---If the plugin has an OnPluginSaveState function this is called before the state is saved - this lets the plugin serialize its script variables - that is, convert from script internal variables to SmushClient variables.
---@return error_code code #
---`error_code.eNotAPlugin`: Script is not currently executing inside a plugin.\
---`error_code.ePluginCouldNotSaveState`: The plugin could not save its state (eg. no "state" directory, disk full, not permitted).\
---`error_code.eOK`: Plugin saved its state OK.
---
---@see Save - save world.
function SaveState() end

---Sets the value of an MXP (custom) entity.
---
---If the supplied contents is the empty string, the entity is deleted.
---
---If the entity already exists it is replaced.
---
---No checking is made for the name or contents strings' values (for example, they might be just spaces).
---
---It is up to you to use sensible values.
---
---Custom entities are stored separately from standard entities. Thus you cannot replace, for example, the "lt" entity.
---@param name string Entity name.
---@param contents string Entity contents, or an empty string to delete the entity.
function SetEntity(name, contents) end

---Sets the contents of a variable. If the named variable already exists the contents are replaced. If the variable doesn't exist it is created. Variables are contained in the "world" configuration, and saved when the world is saved. You can use variables to "remember" data from one session of MUSHclient to the next.
---@param variableName string
---@param contents string
---@return error_code code #
---`error_code.eInvalidObjectLabel`: The variable name is not valid.\
---`error_code.eOK`: The variable contents were set.
function SetVariable(variableName, contents) end
