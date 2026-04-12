---@meta

---Deletes any commands in the command history list for the current window.
---
---@see GetCommandList - get command history.
function DeleteCommandHistory() end

---Discards any commands queued up, to be sent at the speedwalk delay interval.
---@return integer size The number of commands discarded.
---
---@see GetQueue - get queue.
---@see Queue - add to queue.
function DiscardQueue() end

---The `Execute` function lets you execute arbitrary commands as if they were typed into the command window.
---
---The command will be parsed exactly the same as the command window parser, depending on currently-set options (eg. speedwalking, command stacking, scripting etc.).
---
---The only difference is that the command will:
---
---* Not be placed in the command history (as you didn't actually type it in)
---* Not unpause the output window if it is paused.
---
---If you are planning to execute a script using `Execute` you should probably find the current script prefix (using [`GetOption("script_prefix")`](lua://GetOption)) rather than relying it on being the same all the time.
---@param command string
---@return error_code code #
---`error_code.eOK`: Executed OK.\
---`error_code.eWorldClosed`: World is closed.\
---`error_code.eCommandsNestedTooDeeply`: Commands nested too deeply.
function Execute(command) end

---Converts a supplied string based on the rules for "speed walking".
---
---For example:
---
---#4n3esuwdLW
---
---This would move you: 4 times North, 3 times East, South, Up, West, Down and then "lock west"
---
---**Actions**
---
---Actions recognised are: C:close; O:open; L:lock; K:unlock (or nothing for no special action, just walk in that direction)
---
---**Directions**
---
---The directions recognised are: N:north, S:south, E:east, W:west, U:up, D:down.
---
---**Filler**
---
---You can also enter "F" for a speed walk "filler". This is for MUDs that reject multiple identical commands. For example, if you need to walk West 30 times, but your MUD disconnects you after 20 identical commands, enter:
---
---#15w f 15w
---
---The exact text that is sent by the "filler" is customisable from the "Input" world configuration screen. A suggested filler command would be something harmless, like LOOK.
---
---**Multipliers**
---
---You may also enter a one or two-digit multiplier in the range 1 to 99. The multiplier affects the *next* direction entered, e.g. "5s" means go South 5 times.
---
---Spaces are accepted in the speed walk string, and ignored.
---
---**Non-standard directions**
---
---You can supply non-standard directions (eg. NE for NorthEast) by putting them in brackets.
---
---For example:
---
---5N 4W (ne) (say open sesame) 5U 4(se)
---
---You can also supply the "reverse" direction after a "/" character, in case you want to reverse the speedwalk direction later.
---
---For example:
---
---5N 4W (ne/sw) (say open sesame/kick door) 5U 4(se/nw)
---
---When speedwalking, only the characters before the slash are evaluated.
---
---@return string commands The supplied string, converted from speed walk characters to directions, with a "newline" between each direction.
---
---If there is an error in the speed walk string then the first character of the returned string will be an asterisk ("*"), and the rest will be an English error message.
function EvaluateSpeedwalk(speedWalkString) end

---Gets the current contents of the command window, without changing it.
---@return string command Command window contents.
---
---@see GetCommandList - get command history.
function GetCommand() end

---This returns a variant array of the nominated number of most recent commands (in the command history window).
---
---You can get all available commands by requesting a count of nil or 0. (You can get the most recently saved command by requesting a count of 1).
---@param count? integer Number of recent commands to retrieve. If you request more than the available number, then the number of items in the array may be less than *count*.
---@return string[] commands Retrieved commands.
---
---@see GetCommand - get current command (not yet in the command history list).
---@see PushCommand - store in command history.
---@see SendPush - send and store in command history.
function GetCommandList(count) end

---Gets an array of the currently queued commands (speedwalks etc.).
---@return string[] commands Retrieved commands.
---
---@see DiscardQueue - clear queue.
---@see Queue - add to queue.
function GetQueue() end

---Gets the number of milliseconds which elapses between sending speed walk commands (or commands sent with the [`Queue`](lua://Queue) method). Equivalent to [`GetOption("speed_walk_delay)")](lua://GetOption).
---@return integer milliseconds Interval in milliseconds.
---
---@see SetSpeedWalkDelay - setter.
function GetSpeedWalkDelay() end

---This sends the specified text to the world. It is like typing it into the command window.
---
---The message text is also logged to the current log file, if one is open.
---
---The data in the log file may not appear immediately if you view it in another application. To force the data to disk use the [`FlushLog`](lua://FlushLog) function.
---@param ... any Values to concatenate into text.
---@return error_code code #
---`error_code.eWorldClosed`: The world is closed.\
---`error_code.eItemInUse`: Cannot be called from within OnPluginSent script callback.\
---`error_code.eOK`: Sent OK.
---
---@see Send - basic send.
---@see SendImmediate - skip queue.
---@see SendNoEcho - send without echo.
---@see SendPush - send and remember in command history.
---@see SendPkt - send raw bytes.
function LogSend(...) end

---This pastes the supplied text into the command window at the current cursor position, replacing any selection.
---@param command string Text to paste.
---@return string erased The text that it replaced, if any.
---
---@see SetCommand - set command window text.
function PasteCommand(command) end

---Pushes the current contents of the command window into the command history list, and then blanks the current command.
---
---You could use this prior to doing [`SetCommand`](lua://SetCommand), as `SetCommand` will not work with a command in the command window by default.
---@return string erased Text of the command that was in the command window, in case you want to restore it later.
---
---@see SendPush - send and store in command history.
function PushCommand() end

---Queues the supplied command for sending to the MUD at the designated "speed walk" rate. This is for sending commands slowly, because quick sending might be inappropriate.
---
---If the speed walk delay ([`GetSpeedWalkDelay()`](lua://GetSpeedWalkDelay)) is zero, then using `Queue` is exactly the same as using [`Send`](lua://Send).
---
---If *echo* is true then the commands are echoed to the output window (as they are sent, not as they are queued).
---
---If the speed walk delay is not zero, then the supplied message is appended to the end of the command queue. The command is split up at line breaks (whenever a carriage-return/linefeed combination is found), and each line is sent, separated by a delay of SpeedWalkDelay milliseconds.
---
---Periodically (every SpeedWalkDelay milliseconds) the command at the top of the command queue is retrieved and sent to the MUD, and echoed in the output window if required.
---
---If you want to bypass the queue (eg. to say something) then use [`SendImmediate`](lua://SendImmediate).
---@param message string Command to queue.
---@param echo? boolean Echo commands to the output window (as they are sent, not as they are queued). Default: true.
---@return error_code code #
---`error_code.eWorldClosed`: The world is closed.\
---`error_code.eItemInUse`: Cannot be called from within OnPluginSent script callback.\
---`error_code.eOK`: Queued OK.
---
---@see SendImmediate - bypass queue.
function Queue(message, echo) end

---Reverses a supplied string based on the rules for "speed walking". See [`EvaluateSpeedwalk`](lua://EvaluateSpeedwalk) for a detailed description of those rules.
---
---For example:
---
---4n 3e s u w d LW
---
---When reversed, this would become:
---
---LE U E D N 3W 4S
---
---If you supply a "special string" in parentheses, then the order of the words in parentheses is reversed. This lets you do special actions (eg. NE for NorthEast), and supply the reverse direction.
---
---For example:
---
---4n (se/nw)(in/out)
---
---This would become:
---
---(out/in) (nw/se) 4S
---@param speedWalkString string
---@return string speedWalkString Reversed speedwalk.
function ReverseSpeedwalk(speedWalkString) end

---Selects the command in the command window.
function SelectCommand() end

---This sends the specified text to the world. It is similar to typing it into the command window. However aliases, speedwalks, scripting prefixes, command-stacking etc. are not evaluated.
---
---A newline character is appended to the text you supply, as MUD commands normally are terminated with a newline. The text may include embedded newlines. If the text has a trailing newline it is removed. In other words, text with a trailing newline will not end up with two newlines when sent to the MUD.
---
---The command is not saved in the command history buffer (use [`SendPush`](lua://SendPush) if you want to save it).
---@param ... any Values to concatenate into text.
---@return error_code code #
---`error_code.eWorldClosed`: The world is closed.\
---`error_code.eItemInUse`: Cannot be called from within OnPluginSent script callback.\
---`error_code.eOK`: Sent OK
---
---@see LogSend - send and log.
---@see SendImmediate - skip queue.
---@see SendNoEcho - send without echo.
---@see SendPush - send and remember in command history.
---@see SendPkt - send raw bytes.
function Send(...) end

---This sends the specified text to the world. It is like typing it into the command window. If there is a queue of commands in the speedwalk queue, this command "jumps the queue" - that is, it gets sent immediately.
---
---The intention here is to use it for commands (like "say", "tell") that are not dependent on a speedwalk sequence finishing.
---
---A newline character is appended to the text you supply, as MUD commands normally are terminated with a newline. The text may include embedded newlines. If the text has a trailing newline it is removed. In other words, text with a trailing newline will not end up with two newlines when sent to the MUD.
---
---The command is not saved in the command history buffer nor is it logged.
---@param ... any Values to concatenate into text.
---@return error_code code #
---`error_code.eWorldClosed`: The world is closed.\
---`error_code.eItemInUse`: Cannot be called from within OnPluginSent script callback.\
---`error_code.eOK`: Sent OK.
---
---@see LogSend - send and log.
---@see Queue - add commands to the queue.
---@see Send - basic send.
---@see SendNoEcho - send without echo.
---@see SendPush - send and remember in command history.
---@see SendPkt - send raw bytes.
function SendImmediate(...) end

---This sends the specified text to the world. It is like typing it into the command window. The command is not echoed in the output window.
---
---A newline character is appended to the text you supply, as MUD commands normally are terminated with a newline. The text may include embedded newlines. If the text has a trailing newline it is removed. In other words, text with a trailing newline will not end up with two newlines when sent to the MUD.
---
---The command is not saved in the command history buffer (use [`SendPush`](lua://SendPush) if you want to save it).
---@param ... any Values to concatenate into text.
---@return error_code code #
---`error_code.eWorldClosed`: The world is closed.\
---`error_code.eItemInUse`: Cannot be called from within OnPluginSent script callback.\
---`error_code.eOK`: Sent OK
---
---@see LogSend - send and log.
---@see Send - basic send.
---@see SendImmediate - skip queue.
---@see SendPush - send and remember in command history.
---@see SendPkt - send raw bytes.
function SendNoEcho(...) end

---This sends the specified text to the world.
---
---A newline is not appended to it.
---
---If the text contains the IAC character (0xFF) a second IAC character is NOT appended to it.
---
---The text does not go through the OnPluginSend plugin callback, nor through the OnPluginSent callback.
---
---The text is not echoed to the screen.
---
---It is intended only for low-level sending of data for things like telnet negotiation options.
---
---@param packet string
---@return error_code code #
---`error_code.eWorldClosed`: The world is closed.\
---`error_code.eOK`: Sent OK.
---
---@see Send - higher-level send.
function SendPkt(packet) end

---This sends the specified text to the world. It is like typing it into the command window.
---
---A newline character is appended to the text you supply, as MUD commands normally are terminated with a newline. The text may include embedded newlines. If the text has a trailing newline it is removed. In other words, text with a trailing newline will not end up with two newlines when sent to the MUD.
---
---The command is saved in the command history buffer (use [`Send`](lua://Send) if you do NOT want to save it).
---@param ... any Values to concatenate into text.
---@return error_code code #
---`error_code.eWorldClosed`: The world is closed.\
---`error_code.eItemInUse`: Cannot be called from within OnPluginSent script callback.\
---`error_code.eOK`: Sent OK.
---
---@see LogSend - send and log.
---@see Send - basic send.
---@see SendImmediate - skip queue.
---@see SendNoEcho - send without echo.
---@see SendPkt - send raw bytes.
function SendPush(...) end

---This lets you send text to the command window. It will only act if the command window is currently empty (to avoid wiping out something you are typing) unless *force* is `true`.
---@param message string Text to place in the command window.
---@param force? boolean If nil or `false`, this function will fail if the command window is not empty.
---@return error_code code #
---`error_code.eCommandNotEmpty`: Command area was not empty and *force* was not `true`.\
---`error_code.eOK`: Set OK.
---
---@see PasteCommand - replace selected text in the command window.
---@see PushCommand - move current command window text to command history.
function SetCommand(message, force) end

---Lets you select text in the command window. This is intended for specialized applications like a spell checker where, after finding a spelling mistake, you select the misspelt word.
---@param first integer First character to select (1-relative), so the first character is 1. If 0, then any selection is removed.
---@param last integer Last character to select (1-relative). If -1 then the rest of the line is selected.
---@return error_code code #
---`error_code.eOK`: Set OK.
function SetCommandSelection(first, last) end

---Sets the height of the command window in pixels.
---@param height integer Height in pixels.
---@return error_code code #
---`error_code.eBadParameter`: Specified height is < 0 or such that the output window would be < 20 pixels high.\
---`error_code.eOK`: Completed OK.
function SetCommandWindowHeight(height) end

---Use this to change the command window font. The *entire* window font changes, you cannot change individual lines or words.
---@param fontName string Font family name.
---@param pointSize number Font point size.
---@param weight? integer Common values: 300 (light), 400 (normal), 700 (bold). Use nil or 0 to leave unspecified.
---@param italic? boolean Use italic font. Default: `false`.
---
---@see AddFont - import a font file.
---@see SetOutputFont - set font of MUD output.
---@see WindowFont - set font of miniwindow.
function SetInputFont(fontName, pointSize, weight, italic) end

---This property is used to set the number of milliseconds which elapses between sending speed walk commands (or commands sent with the [`Queue`](lua://Queue) method). Equivalent to [`SetOption("speed_walk_delay", speedWalkDelay)`](lua://SetOption).
---
---If the interval is 0, then commands are not queued, but are sent immediately, as fast as possible.
---
---Setting the interval to 0, when it was previously some other value, flushes any outstanding commands from the queue, so they are sent immediately.
---@param speedWalkDelay integer Interval in milliseconds.
---
---@see GetSpeedWalkDelay - getter.
function SetSpeedWalkDelay(speedWalkDelay) end
