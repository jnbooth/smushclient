---@meta

---Creates an alias.
---@param aliasName string Name of this alias - may be empty.
---@param matchText string What to match on.
---@param responseText string What to send to the world.
---@param flags integer OR combination of [`alias_flag`](lua://alias_flag) values:
---`alias_flag.Enabled` (0x1): Enable\
---`alias_flag.KeepEvaluating` (0x8): Keep evaluating\
---`alias_flag.IgnoreAliasCase` (0x20): Ignore case when matching\
---`alias_flag.OmitFromLogFile` (0x40): Omit from log file\
---`alias_flag.RegularExpression` (0x80): Uses regular expression\
---`alias_flag.ExpandVariables` (0x200): Expand variables like @direction\
---`alias_flag.Replace` (0x400): Replace existing of same name\
---`alias_flag.AliasSpeedWalk` (0x800): Interpret send string as a speed walk string\
---`alias_flag.AliasQueue` (0x1000): Queue this alias for sending at the speedwalking delay interval\
---`alias_flag.AliasMenu` (0x2000): This alias appears on the alias menu\
---`alias_flag.Temporary` (0x4000): Temporary - do not save to world file\
---`alias_flag.OneShot` (0x8000): If set, only fires once
---@param scriptName? string If neither nil nor empty, which script subroutine to execute.
---@return error_code code #
---`error_code.eInvalidObjectLabel`: The alias name is not valid.\
---`error_code.eAliasAlreadyExists`: A alias of that name already exists.\
---`error_code.eAliasCannotBeEmpty`: The "match_text" cannot be empty.\
---`error_code.eScriptNameNotLocated`: The script name cannot be located in the script file.\
---`error_code.eBadRegularExpression`: The regular expression could not be evaluated.\
---`error_code.eOK`: Added OK.
---
---@see AddTimer
---@see AddTrigger
---@see DeleteAlias
function AddAlias(aliasName, matchText, responseText, flags, scriptName) end

---Adds a timer to the list of timers.
---
---Timers can be used to cause something to happen periodically, either every x seconds, or at a certain time of day.
---
---Periodical timers (which fire after a certain time has elapsed) are intended for periodical events (eg. save your world file every hour, drink a potion every minute, check your health every 10 seconds).
---
---Timers that fire at a certain time of day are intended to do things like start a new log file at midnight, warn you if when it is time to go to work, that sort of thing. :)
---
---You can make a "one-shot" timer that does something once, and then deletes itself. This is useful for making a single thing happen in the future (eg. in 5 minutes) rather than happening every 5 minutes.
---@param timerName string Name of this timer - may be empty.
---@param hour integer Hour to fire (0-23).
---@param minute integer Minute to fire (0-59).
---@param second number Second to fire (0-59.9999).
---@param responseText string What to send to the world.
---@param flags integer OR combination of [`timer_flag`](lua://timer_flag) values:
---`timer_flag.Enable` (0x1): Enable\
---`timer_flag.AtTime` (0x2): If not set, time is "every"\
---`timer_flag.OneShot` (0x4): If set, only fires once\
---`timer_flag.TimerSpeedWalk` (0x8): Timer does a speed walk when it fires\
---`timer_flag.TimerNote` (0x10): Timer does a [`Note`](lua://Note) when it fires
---`timer_flag.ActiveWhenClosed` (0x20): Timer fires even when world is disconnected
---`timer_flag.Replace` (0x400): Replace existing of same name
---`timer_flag.Temporary` (0x4000): Temporary - do not save to world file
---`timer_flag.OneShot` (0x8000): If set, only fires once
---@param scriptName? string If neither nil nor empty, which script subroutine to execute.
---@return error_code code #
---`error_code.eInvalidObjectLabel`: The timer name is not valid.\
---`error_code.eTimerAlreadyExists`: A timer of that name already exists.\
---`error_code.eScriptNameNotLocated`: The script name cannot be located in the script file.\
---`error_code.eTimeInvalid`: The time is invalid (eg. hour not in range 0 to 23).\
---`error_code.eOK`: Added OK.
---
---@see AddAlias
---@see AddTrigger
---@see DeleteTimer
---@see DoAfter
---@see DoAfterNote
---@see DoAfterSpeedwalk
---@see DoAfterSpecial
function AddTimer(timerName, hour, minute, second, responseText, flags, scriptName) end

---Adds a trigger to the list of triggers.
---@param triggerName string Name of this trigger - may be empty.
---@param matchText string What to match on.
---@param responseText string What to send back to the world.
---@param flags integer OR combination of [`trigger_flag`](lua://trigger_flag) values:
---`trigger_flag.Enabled` (0x1): Enable\
---`trigger_flag.OmitFromLog` (0x2): Omit from log file\
---`trigger_flag.OmitFromOutput` (0x4): Omit from output\
---`trigger_flag.KeepEvaluating` (0x8): Keep evaluating\
---`trigger_flag.IgnoreCase` (0x10): Ignore case when matching\
---`trigger_flag.RegularExpression` (0x20): Uses regular expression\
---`trigger_flag.ExpandVariables` (0x200): Expand variables like @direction\
---`trigger_flag.Replace` (0x400): Replace existing of same name\
---`trigger_flag.LowercaseWildcard` (0x800): Wildcards forced to lower-case\
---`trigger_flag.Temporary` (0x4000): Temporary - do not save to world file\
---`trigger_flag.OneShot` (0x8000): If set, only fires once
---@param colour? custom_colour Foreground colour. See [`custom_colour`](lua://custom_colour). Default: `custom_colour.NoChange` (-1). Note: custom colours are only partially supported by SmushClient. RGB colour codes are preferred.
---@param wildcard? integer If neither nil nor 0, copy this wildcard to the clipboard.
---@param soundFileName? string If neither nil nor empty, play a sound file when triggered.
---@param scriptName? string If neither nil nor empty, which script subroutine to trigger.
---@param sendTo? sendto Where to send the trigger text. See [`sendto`](lua://sendto). Default: `sendto.world`.
---@param sequence? integer Sequence number of trigger (lower is evaluated earlier). Default: 100.
---@return error_code code #
---`error_code.eInvalidObjectLabel`: The trigger name is not valid.\
---`error_code.eTriggerAlreadyExists`: A trigger of that name already exists.\
---`error_code.eTriggerCannotBeEmpty`: The "match_text" cannot be empty.\
---`error_code.eScriptNameNotLocated`: The script name cannot be located in the script file.\
---`error_code.eBadRegularExpression`: The regular expression could not be evaluated.\
---`error_code.eTriggerSequenceOutOfRange`: Sequence is not in range 0 to 10000.\
---`error_code.eTriggerSendToInvalid`: Send to field is not in range 0 to 9.\
---`error_code.eOK`: Added OK.
function AddTrigger(triggerName, matchText, responseText, flags, colour, wildcard, soundFileName, scriptName, sendTo, sequence) end

---Alias for [`AddTrigger`](lua://AddTrigger) for compatibility with MUSHclient.
AddTriggerEx = AddTrigger

---Deletes an alias by name.
---
---WARNING - if the alias is executing a script it cannot be deleted. For example, an alias cannot delete itself.
---
---If you need to delete an alias from within itself use [`DoAfterSpecial`](lua://DoAfterSpecial) to delete it after a short delay. You might also want to use [`EnableAlias`](lua://EnableAlias) to disable it first.
---@param aliasName string
---@return error_code code #
---`error_code.eInvalidObjectLabel`: The alias name is not valid.\
---`error_code.eAliasNotFound`: The named alias does not exist.\
---`error_code.eItemInUse`: Cannot delete trigger/alias/timer because it is executing a script.\
---`error_code.eOK`: Deleted OK.
---
---@see AddAlias
---@see DeleteAliasGroup
---@see DeleteTimer
---@see DeleteTrigger
---@see EnableAlias
function DeleteAlias(aliasName) end

---Triggers, aliases and timers can be placed in "groups" where a group is a bunch of related items. Groups have names, there is no restriction on the name of a group. Ungrouped items have blank (empty) names.
---
---To delete an entire group of aliases do this:
---
---```lua
---DeleteAliasGroup("groupname")
---```
---
---For debugging purposes the function returns a count of the number of members deleted. eg.
---
---```lua
---local mycount = world.DeleteAliasGroup("groupname")
---```
---
---If you are using plugins only the members of the current plugin (if any) are deleted. If the call is made outside a plugin only the members of the group that are not in a plugin are deleted.
---
---WARNING - if the alias is executing a script it cannot be deleted. For example, an alias cannot delete itself.
---
---If you need to delete an alias from within itself use [`DoAfterSpecial`](lua://DoAfterSpecial) to delete it after a short delay. You might also want to use [`EnableAliasGroup`](lua://EnableAliasGroup) to disable it first.
---
---Since `DeleteAliasGroup` deletes multiple aliases, any found to be "in use" (that is, executing a script) will not be deleted.
---@param groupName string
---@return integer deleted Number of deleted aliases.
---
---@see DeleteAlias
---@see DeleteTimerGroup
---@see DeleteTriggerGroup
---@see EnableAliasGroup
function DeleteAliasGroup(groupName) end

---Deletes the named timer from the list of timers.
---
---WARNING - if the timer is executing a script it cannot be deleted. For example, a timer cannot delete itself.
---
---If you need to delete a timer from within itself use [`DoAfterSpecial`](lua://DoAfterSpecial) to delete it after a short delay. You might also want to use [`EnableTimer`](lua://EnableTimer) to disable it first.
---@param timerName string
---@return error_code code #
---`error_code.eInvalidObjectLabel`: The timer name is not valid.\
---`error_code.eTimerNotFound`: The named timer does not exist.\
---`error_code.eItemInUse`: Cannot delete trigger/alias/timer because it is executing a script.\
---`error_code.eOK`: Deleted OK.
---
---@see AddTimer
---@see DeleteAlias
---@see DeleteTimerGroup
---@see DeleteTrigger
---@see EnableTimer
function DeleteTimer(timerName) end

---Triggers, aliases and timers can be placed in "groups" where a group is a bunch of related items. Groups have names, there is no restriction on the name of a group. Ungrouped items have blank (empty) names.
---
---To delete an entire group of timers do this:
---
---```lua
---DeleteTimerGroup("groupname")
---
---
---For debugging purposes the function returns a count of the number of members deleted. eg.
---
---```lua
---local mycount = DeleteTimerGroup("groupname")
---```
---
---If you are using plugins only the members of the current plugin (if any) are deleted. If the call is made outside a plugin only the members of the group that are not in a plugin are deleted.
---
---WARNING - if the timer is executing a script it cannot be deleted. For example, a timer cannot delete itself.
---
---If you need to delete a timer from within itself use [`DoAfterSpecial`](lua://DoAfterSpecial) to delete it after a short delay. You might also want to use [`EnableTimerGroup`](lua://EnableTimerGroup) to disable it first.
---
---Since `DeleteTimerGroup` deletes multiple timers, any found to be "in use" (that is, executing a script) will not be deleted.
---@param groupName string
---@return integer deleted Number of deleted timers.
---
---@see DeleteAliasGroup
---@see DeleteTimer
---@see DeleteTriggerGroup
---@see EnableTimerGroup
function DeleteTimerGroup(groupName) end

---Deletes the named trigger from the list of triggers.
---WARNING - if the trigger is executing a script it cannot be deleted. For example, a trigger cannot delete itself.
---
---If you need to delete a trigger from within itself use [`DoAfterSpecial`](lua://DoAfterSpecial) to delete it after a short delay. You might also want to use [`EnableTrigger`](lua://EnableTrigger) to disable it first.
---@param triggerName string
---@return error_code code #
---`error_code.eInvalidObjectLabel`: The trigger name is not valid.\
---`error_code.eTriggerNotFound`: The named trigger does not exist.\
---`error_code.eItemInUse`: The trigger is currently executing a script.\
---`error_code.eOK`: Deleted OK.
---
---@see AddTrigger
---@see DeleteAlias
---@see DeleteTimer
---@see DeleteTriggerGroup
---@see EnableTrigger
function DeleteTrigger(triggerName) end

---Triggers, aliases and timers can be placed in "groups" where a group is a bunch of related items. Groups have names, there is no restriction on the name of a group. Ungrouped items have blank (empty) names.
---
---To delete an entire group of triggers do this:
---
---```lua
---DeleteTriggerGroup("groupname")
---```
---
---For debugging purposes the function returns a count of the number of members deleted. eg.
---
---```lua
---local mycount = DeleteTriggerGroup("groupname")
---```
---
---If you are using plugins only the members of the current plugin (if any) are deleted. If the call is made outside a plugin only the members of the group that are not in a plugin are deleted.
---
---WARNING - if the trigger is executing a script it cannot be deleted. For example, a trigger cannot delete itself.
---
---If you need to delete a trigger from within itself use [`DoAfterSpecial`](lua://DoAfterSpecial) to delete it after a short delay. You might also want to use [`EnableTriggerGroup`](lua://EnableTriggerGroup) to disable it first.
---
---Since `DeleteTriggerGroup` deletes multiple triggers, any found to be "in use" (that is, executing a script) will not be deleted.
---@param groupName string
---@return integer deleted Number of deleted triggers.
---
---@see DeleteAliasGroup
---@see DeleteTimerGroup
---@see DeleteTrigger
---@see EnableTriggerGroup
function DeleteTriggerGroup(groupName) end

---Alias for [`DoAfterSpecial(seconds, sendText, sendto.world)`](lua://DoAfterSpecial).
---@param seconds number Delay in seconds.
---@param sendText string Command content.
---@return error_code code #
---`error_code.eTimeInvalid`: The time is invalid (seconds not in range 0.1 to 86,399).\
---`error_code.eOK`: Added OK.
function DoAfter(seconds, sendText) end

---Alias for [`DoAfterSpecial(seconds, sendText, sendto.output)`](lua://DoAfterSpecial).
---@param seconds number Delay in seconds.
---@param sendText string Note content.
---@return error_code code #
---`error_code.eTimeInvalid`: The time is invalid (seconds not in range 0.1 to 86,399).\
---`error_code.eOK`: Added OK.
function DoAfterNote(seconds, sendText) end

---This routine adds an unlabelled, temporary, one-shot timer, set to go off after the designated number of seconds.
---
---The timer sends its *sendText* text to the designated location when it fires.
---
---The purpose of this is simplify the frequent case of simply wanting to do something in a few seconds, rather than using [`AddTimer`](lua://AddTimer).
---
---This lets you do things that involve simple scripting at some future time, without having to go to a lot of trouble writing script routines.
---
---The case "set variable" is not very useful because there is no way of specifying which variable, however instead you could use "send to script" (12) and do a "setvariable". eg.
---
---```lua
---DoAfterSpecial(5, "SetVariable ""target"", ""kobold"" ", sendto.script)
---```
---
---This example uses double-quotes for the variable name and variable value, because they are quotes within quotes (an argument to [`SetVariable`](lua://SetVariable) inside the argument to `DoAfterSpecial`).
---
---Also, case "Script - after omit" (`sendto.scriptafteromit`) is really intended for use in triggers, and will have the same behaviour as "Script" (`sendto.script`) if used elsewhere.
---
---WARNING - as `DoAfterSpecial` is implemented by using temporary MUSHclient timers, it will not work if timers are disabled. If `DoAfterSpecial` is not working for you (however the function call is returning zero) then ensure that timers are enabled in the Timers configuration dialog.
---@param seconds number Delay in seconds in between 0.1 second and 23 hours 59 minutes 59 seconds (that is, 0.1 to 86,399).
---@param sendText string Action content.
---@param target sendto Action send target. See [`sendto`](lua://sendto).
---@return error_code code #
---`error_code.eTimeInvalid`: The time is invalid (seconds not in range 0.1 to 86,399).\
---`error_code.eOptionOutOfRange`: The "SendTo" argument is not in the range 0 to 12.\
---`error_code.eOK`: Added OK.
function DoAfterSpecial(seconds, sendText, target) end

---Alias for [`DoAfterSpecial(seconds, sendText, sendto.speedwalk)`](lua://DoAfterSpecial).
---@param seconds number Delay in seconds.
---@param text string Command content.
---@return error_code code #
---`error_code.eTimeInvalid`: The time is invalid (seconds not in range 0.1 to 86,399).\
---`error_code.eOK`: Added OK.
function DoAfterSpeedwalk(seconds, text) end

---Enables or disables the named alias. An enabled alias is "active", otherwise it is ignored when processing commands you type.
---@param aliasName string Alias name.
---@param enabled? boolean Default: `true`.
---@return error_code code #
---`error_code.eInvalidObjectLabel`: The alias name is not valid.\
---`error_code.eAliasNotFound`: The named alias does not exist.\
---`error_code.eOK`: Completed OK.
---
---@see DeleteAlias
---@see EnableAliasGroup
---@see EnableTimer
---@see EnableTrigger
function EnableAlias(aliasName, enabled) end

---Triggers, aliases and timers can be placed in "groups" where a group is a bunch of related items. Groups have names, there is no restriction on the name of a group. Ungrouped items have blank (empty) names.
---
---To enable an entire group of aliases do this:
---
---```lua
---EnableAliasGroup("groupname", true)
---```
---
---To disable the entire group:
---
---```lua
---EnableAliasGroup("groupname", false)
---```
---
---For debugging purposes the function returns a count of the number of members in the group (not the number enabled, as some may already be enabled). eg.
---
---```lua
---local mycount = EnableAliasGroup("groupname", true)
---```
---
---If you are using plugins only the members of the current plugin (if any) are affected. If the call is made outside a plugin only the members of the group that are not in a plugin are affected.
---@param groupName string Group name.
---@param enabled? boolean Default: `true`.
---@return integer aliases Count of the number of items in that group.
---
---@see DeleteAliasGroup
---@see EnableAlias
---@see EnableGroup
---@see EnableTimerGroup
---@see EnableTriggerGroup
function EnableAliasGroup(groupName, enabled) end

---Triggers, aliases and timers can be placed in "groups" where a group is a bunch of related items. Groups have names, there is no restriction on the name of a group. Ungrouped items have blank (empty) names.
---
---To enable an entire group of triggers, aliases and timers do this:
---
---```lua
---EnableGroup("groupname", true)
---```
---
---To disable the entire group:
---
---```lua
---EnableGroup("groupname", false)
---```
---
---For debugging purposes the function returns a count of the number of members in the group (not the number enabled, as some may already be enabled). eg.
---
---```lua
---local mycount = EnableGroup("groupname", true)
---```
---
---If you are using plugins only the members of the current plugin (if any) are affected. If the call is made outside a plugin only the members of the group that are not in a plugin are affected.
---@param groupName string Group name.
---@param enabled? boolean Default: `true`.
---@return integer members Count of the number of items in that group.
---
---@see EnableAliasGroup
---@see EnableTimerGroup
---@see EnableTriggerGroup
function EnableGroup(groupName, enabled) end

---Enables or disables the named timer. An enabled timer is "active", otherwise it is ignored when processing periodic events.
---@param timerName string Timer name.
---@param enable? boolean Default: `true`.
---@return error_code code #
---`error_code.eInvalidObjectLabel`: The timer name is not valid.\
---`error_code.eTimerNotFound`: The named timer does not exist.\
---`error_code.eOK`: Completed OK.
---
---@see DeleteTimer
---@see EnableAlias
---@see EnableTimerGroup
---@see EnableTrigger
function EnableTimer(timerName, enable) end

---Triggers, aliases and timers can be placed in "groups" where a group is a bunch of related items. Groups have names, there is no restriction on the name of a group. Ungrouped items have blank (empty) names.
---
---To enable an entire group of timers do this:
---
---```lua
---EnableTimerGroup("groupname", true)
---
---To disable the entire group:
---
---```lua
---EnableTimerGroup("groupname", false)
---```
---
---For debugging purposes the function returns a count of the number of members in the group (not the number enabled, as some may already be enabled). eg.
---
---```lua
---lua mycount = EnableTimerGroup("groupname", true)
---```
---
---If you are using plugins only the members of the current plugin (if any) are affected. If the call is made outside a plugin only the members of the group that are not in a plugin are affected.
---@param groupName string Group name.
---@param enabled? boolean Default: `true`.
---@return integer timers Count of the number of items in that group.
---
---@see DeleteTimerGroup
---@see EnableAliasGroup
---@see EnableGroup
---@see EnableTimer
---@see EnableTriggerGroup
function EnableTimerGroup(groupName, enabled) end

---Enables or disables the named trigger. An enabled trigger is "active", otherwise it is ignored when processing incoming text from the MUD.
---@param triggerName string Trigger name.
---@param enabled? boolean Default: `true`.
---@return error_code code #
---`error_code.eInvalidObjectLabel`: The trigger name is not valid.\
---`error_code.eTriggerNotFound`: The named trigger does not exist.\
---`error_code.eOK`: Completed OK.
---
---@see DeleteTrigger
---@see EnableAlias
---@see EnableTimer
---@see EnableTriggerGroup
function EnableTrigger(triggerName, enabled) end

---Triggers, aliases and timers can be placed in "groups" where a group is a bunch of related items. Groups have names, there is no restriction on the name of a group. Ungrouped items have blank (empty) names.
---
---To enable an entire group of triggers do this:
---
---```lua
---EnableTriggerGroup("groupname", true)
---```
---
---To disable the entire group:
---
---```lua
---EnableTriggerGroup("groupname", false)
---```
---
---For debugging purposes the function returns a count of the number of members in the group (not the number enabled, as some may already be enabled). eg.
---
---```lua
---local mycount = EnableTriggerGroup("groupname", true)
---```
---
---If you are using plugins only the members of the current plugin (if any) are affected. If the call is made outside a plugin only the members of the group that are not in a plugin are affected.
---@param groupName string Group name.
---@param enabled? boolean Default: `true`.
---@return integer triggers A count of the number of items in that group.
function EnableTriggerGroup(groupName, enabled) end

---Gets details about the specified alias.
---
---For a detailed description about the meanings of the fields, see [`AddAlias`](lua://AddAlias).
---@param aliasName string Alias name.
---@return error_code code #
---`error_code.eInvalidObjectLabel`: The alias name is not valid.\
---`error_code.eAliasNotFound`: The named alias does not exist.\
---`error_code.eOK`: Completed OK.
---@return string? match
---@return string? response
---@return integer? flags
---@return string? scriptName
function GetAlias(aliasName) end

---Returns an array of all the named aliases currently defined. You can then use [`GetAlias`](lua://GetAlias) or [`GetAliasInfo`](lua://GetAliasInfo) for more details about each one.
---@return string[] aliasNames
---
---@see GetPluginAliasList
---@see GetTimerList
---@see GetTriggerList
function GetAliasList() end

---Once a alias has matched you can extract any of the wildcards for that alias using this function.
---
---The wildcards can be specified by name or number (if by number pass a string which is the appropriate number).
---
---You can set up a named wildcard like this:
---
---Match: `Tell (?P<who>.*?) (?P<what>.*)`
---
---In this case we have 2 wildcards, named "who" and "what".
---
---Now we can extract wildcards like this:
---
---```lua
---x = GetAliasWildcard("myalias", "who")  -- get wildcard named 'who'
---x = GetAliasWildcard("myalias", "what") -- get wildcard named 'what'
---x = GetAliasWildcard("myalias", "22")   -- get wildcard 22 (if it exists)
---```
---
---You can have up to 999 wildcards, plus wildcard 0 which represents the entire matching string.
---
---You can use [`GetAliasInfo(24)`](lua://GetAliasInfo) to find how many wildcards matched.
---
---Each wildcard will have a number and may optionally have a name.
---@param aliasName string
---@param wildcardName string
---@return string|nil wildcard
---
---@see GetTriggerWildcard
function GetAliasWildcard(aliasName, wildcardName) end

---Returns an array of all the named aliases currently defined for the nominated plugin. You can then use [`GetPluginAliasInfo`](lua://GetPluginAliasInfo) to find out more details about each one.
---
---If you want to find the list of aliases in the current plugin, use [`GetAliasList`](lua://GetAliasList).
---
---If you are writing a plugin and want to find the "global" MUSHclient alias list, use an empty plugin ID, eg.
---
---```lua
---local alList = GetPluginAliasList("")
---```
---@param pluginID string
---@return string[] aliasNames
---
---@see GetAliasList
---@see GetPluginTimerList
---@see GetPluginTriggerList
function GetPluginAliasList(pluginID) end

---Returns an array of all the named timers currently defined for the nominated plugin. You can then use [`GetPluginTimerInfo`](lua://GetPluginTimerInfo) to find out more details about each one.
---
---If you want to find the list of timers in the current plugin, use [`GetTimerList`](lua://GetTimerList).
---
---If you are writing a plugin and want to find the "global" MUSHclient timer list, use an empty plugin ID, eg.
---
---```lua
---local tList = GetPluginTimerList("")
---```
---@param pluginID string
---@return string[] timerNames
---
---@see GetPluginAliasList
---@see GetPluginTriggerList
---@see GetTimerList
function GetPluginTimerList(pluginID) end

---Returns an array of all the named triggers currently defined for the nominated plugin. You can then use [`GetPluginTriggerInfo`](lua://GetPluginTriggerInfo) to find out more details about each one.
---
---If you want to find the list of triggers in the current plugin, use [`GetTriggerList`](lua://GetTriggerList).
---
---If you are writing a plugin and want to find the "global" MUSHclient trigger list, use an empty plugin ID, eg.
---
---```lua
---local trList = GetPluginTriggerList("")
---```
---@param pluginID string
---@return string[] triggerNames
---
---@see GetPluginAliasList
---@see GetPluginTimerList
---@see GetTriggerList
function GetPluginTriggerList(pluginID) end

---Gets details about the specified timer.
---
---For a detailed description about the meanings of the fields, see [`AddTimer`](lua://AddTimer).
---@param timerName string Timer name.
---@return error_code code #
---`error_code.eInvalidObjectLabel`: The timer name is not valid.\
---`error_code.eTimerNotFound`: The named timer does not exist.\
---`error_code.eOK`: Completed OK.
---@return integer? hour
---@return integer? minute
---@return number? second
---@return string? response
---@return integer? flags
---@return string? scriptName
function GetTimer(timerName) end

---Returns an array of all the named timers currently defined. You can then use [`GetTimer`](lua://GetTimer) or [`GetTimerInfo`](lua://GetTimerInfo) for more details about each one.
---@return string[] timerNames
---
---@see GetAliasList
---@see GetPluginTimerList
---@see GetTriggerList
function GetTimerList() end

---Gets details about the specified trigger.
---
---For a detailed description about the meanings of the fields, see [`AddTrigger`](lua://AddTrigger).
---@param triggerName string Trigger name.
---@return error_code code #
---`error_code.eInvalidObjectLabel`: The trigger name is not valid.\
---`error_code.eTriggerNotFound`: The named trigger does not exist.\
---`error_code.eOK`: Completed OK.
---@return string? match
---@return string? response
---@return integer? flags
---@return -1? colour
---@return string? soundFileName
---@return string? scriptName
function GetTrigger(triggerName) end

---Returns an array of all the named triggers currently defined. You can then use [`GetTrigger`](lua://GetTrigger) or [`GetTriggerInfo`](lua://GetTriggerInfo) to find out more details about each one.
---
---If `GetTriggerList` is called from within a plugin, the triggers for the current plugin are used, not the "global" MUSHclient triggers.
---
---If you want to find the list of triggers in another plugin, or the global MUSHclient triggers, use [`GetPluginTriggerList`](lua://GetPluginTriggerList).
---@return string[] triggerNames
---
---@see GetAliasList
---@see GetPluginTriggerList
---@see GetTimerList
function GetTriggerList() end

---Once a trigger has matched you can extract any of the wildcards for that trigger using this function.
---
---The wildcards can be specified by name or number (if by number pass a string which is the appropriate number).
---
---You can set up a named wildcard like this:
---
---Match: `(?P<who>.*) tells you '(?P<what>.*)'`
---
---In this case we have 2 wildcards, named "who" and "what".
---
---Now we can extract wildcards like this:
---
---```lua
---x = GetTriggerWildcard("mytrigger", "who")  -- get wildcard named 'who'
---x = GetTriggerWildcard("mytrigger", "what") -- get wildcard named 'what'
---x = GetTriggerWildcard("mytrigger", "22")   -- get wildcard 22 (if it exists)
---```
---
---You can have up to 999 wildcards, plus wildcard 0 which represents the entire matching string.
---
---You can use [`GetTriggerInfo(31)`](lua://GetTriggerInfo) to find how many wildcards matched.
---
---Each wildcard will have a number and may optionally have a name.
---@param triggerName string
---@param wildcardName string
---@return string|nil wildcard
function GetTriggerWildcard(triggerName, wildcardName) end

---Used to test if an alias exists.
---
---Note that this function returns 0 if the alias *exists*.
---@param name string Alias name.
---@return error_code code #
---`error_code.eInvalidObjectLabel`: The alias name is not valid.\
---`error_code.eAliasNotFound`: The named alias does not exist.\
---`error_code.eOK`: completed OK - ie. The alias exists.
---
---@see IsTimer
---@see IsTrigger
function IsAlias(name) end

---Used to test if a timer exists.
---
---Note that this function returns 0 if the timer *exists*.
---@param name string Timer name.
---@return error_code code #
---`error_code.eInvalidObjectLabel`: The timer name is not valid.\
---`error_code.eTimerNotFound`: The named timer does not exist.\
---`error_code.eOK`: completed OK - ie. The timer exists.
---
---@see IsAlias
---@see IsTrigger
function IsTimer(name) end

---Used to test if a trigger exists.
---
---Note that this function returns 0 if the trigger *exists*.
---@param name string Trigger name.
---@return error_code code #
---`error_code.eInvalidObjectLabel`: The trigger name is not valid.\
---`error_code.eTriggerNotFound`: The named trigger does not exist.\
---`error_code.eOK`: completed OK (ie. The trigger exists)
---
---@see IsAlias
---@see IsTimer
function IsTrigger(name) end

---Call this function from a trigger script to stop trigger evaluation. This would have to be called from "send to script" (not send-to-script-after-omit), nor from a trigger script file, in order to be effective. If the argument is true, then no further trigger evaluation takes place, in any further plugins.
---
---Note that since calling a trigger script (script name in Script box) is done after all triggers are evaluated for that plugin, doing this inside a script file will be too late (for the current plugin). It has to be done in send-to-script.
function StopEvaluatingTriggers() end
