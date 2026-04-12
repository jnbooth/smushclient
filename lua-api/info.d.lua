---@meta

---@alias ActionSource
---| 0 # No particular reason, could be plugin saving
---| 1 # User typed something in the command area and pressed <Enter>
---| 3 # User used the numeric keypad
---| 5 # Item chosen from pop-up menu
---| 6 # Trigger fired
---| 7 # Timer fired
---| 8 # Input arrived (eg. packet received)
---| 9 # Some sort of world action (e.g. world open, connect, got focus)
---| 11 # Hotspot callback

---@alias ConnectPhase
---| 0 # Disconnected
---| 1 # Host lookup
---| 3 # Connecting
---| 8 # Connected

---@alias LineAction
---| 0 # None
---| 1 # Send to MUD
---| 2 # Hyperlink
---| 3 # Prompt

---@alias WindowState
---| 0 # Hidden
---| 1 # Normal
---| 3 # Maximized
---| 6 # Minimized

---Gets *boolean* information about an alias.
---@param aliasName string
---@param infoType
---| 4 # Omit from log
---| 5 # Omit from output
---| 6 # Enabled
---| 7 # Regular expression
---| 8 # Ignore case
---| 9 # Expand variables
---| 12 # Display in alias menu
---| 14 # Temporary
---| 19 # Keep evaluating
---| 21 # Echo
---| 22 # Omit from command history
---| 27 # Script is valid
---| 29 # One-shot
---@return boolean info
---
---@see GetPluginAliasInfo - get information about an alias from another plugin.
---@see GetTimerInfo - get information about a timer.
---@see GetTriggerInfo - get information about a trigger.
function GetAliasInfo(aliasName, infoType) end

---Gets the send-to location of an alias.
---@param aliasName string
---@param infoType
---| 18 # Send-to location
---@return sendto info
---
---@see GetPluginAliasInfo - get information about an alias from another plugin.
---@see GetTimerInfo - get information about a timer.
---@see GetTriggerInfo - get information about a trigger.
function GetAliasInfo(aliasName, infoType) end

---Gets *integer* information about an alias.
---@param aliasName string
---@param infoType
---| 20 # Sequence number
---| 23 # User option value
---@return integer info
---
---@see GetPluginAliasInfo - get information about an alias from another plugin.
---@see GetTimerInfo - get information about a timer.
---@see GetTriggerInfo - get information about a trigger.
function GetAliasInfo(aliasName, infoType) end

---Gets *string* information about an alias.
---@param aliasName string
---@param infoType
---| 1 # What to match on
---| 2 # What to send
---| 3 # Script procedure name
---| 16 # Group name
---| 17 # Variable name to set
---@return string info
---
---@see GetPluginAliasInfo - get information about an alias from another plugin.
---@see GetTimerInfo - get information about a timer.
---@see GetTriggerInfo - get information about a trigger.
function GetAliasInfo(aliasName, infoType) end


---Gets *string* information about the current world.
---@param infoType
---| 1 # Server name (IP address)
---| 2 # World name
---| 3 # Character name
---| 9 # Sound on new activity
---| 11 # Log file preamble
---| 12 # Log file postamble
---| 13 # Log file line preamble - player input
---| 14 # Log file line preamble - notes
---| 15 # Log file line preamble - MUD output
---| 16 # Log file line postamble - player input
---| 17 # Log file line postamble - notes
---| 18 # Log file line postamble - MUD output
---| 19 # Speed Walk Filler
---| 20 # Output window font name
---| 21 # Speed walk prefix
---| 22 # Text sent on connecting
---| 23 # Input font name
---| 28 # Scripting language ("lua")
---| 35 # Script file name
---| 40 # Auto-log file name
---| 42 # Terminal ID (telnet negotiation)
---| 50 # Sound to play for beeps
---| 51 # Current log file name
---| 53 # Current status line message
---| 54 # World file pathname
---| 55 # World title
---| 56 # MUSHclient application path name
---| 57 # World files default path (directory)
---| 58 # Log files default path (directory)
---| 59 # Script files default path (directory)
---| 60 # Plugin files default path (directory)
---| 61 # World TCP/IP address (as string, after DNS lookup, eg. 10.0.0.16)
---| 62 # Proxy server TCP/IP address (as string, after DNS lookup, eg. 10.0.0.22)
---| 63 # Host name (name of this PC)
---| 64 # Current directory
---| 65 # Script function for world save ("Save")
---| 66 # MUSHclient application directory
---| 67 # World file directory
---| 68 # MUSHclient startup (initial) directory
---| 69 # Translation file
---| 70 # Locale
---| 71 # Font used for fixed-pitch dialogs
---| 72 # MUSHclient version (eg. "4.11")
---| 73 # MUSHclient compilation date/time (eg. "Jul 25 2007 14:21:10")
---| 74 # Default sounds file directory (presently GetInfo (66) + "sounds/")
---| 75 # Last telnet subnegotiation string received
---| 77 # Windows version debug string (eg. "Service Pack 2")
---| 78 # Foreground image name
---| 79 # Background image name
---| 82 # Preferences database pathname
---| 83 # SQLite3 database version
---| 84 # File-browsing directory
---| 86 # Word under mouse on mouse menu click
---| 87 # Last command sent to the MUD
---@return string info
function GetInfo(infoType) end

---Gets *boolean* information about the current world.
---@param infoType
---| 101 # 'No Echo' flag
---| 103 # Decompressing
---| 104 # MXP active
---| 105 # Pueblo active (false; unsupported)
---| 106 # Disconnected flag (true if not connected)
---| 107 # Currently-connecting flag
---| 108 # OK-to-disconnect flag
---| 111 # 'World file is modified' flag
---| 112 # Automapper active flag (false; unsupported)
---| 113 # 'World is active' flag
---| 114 # 'Output window paused' flag
---| 118 # Variables have changed
---| 119 # Script engine is active (true)
---| 120 # Scoll bar is visible for output window
---| 121 # High-resolution timer is available (true)
---| 122 # Is the SQLite3 library thread-safe?
---| 123 # Are we currently processing a "Simulate" function call?
---| 124 # Is the current line from the MUD being omitted from output? (false)
---| 125 # Is the client in full-screen mode?
---| 285 # Is output window available? (true or false)
---@return boolean info
function GetInfo(infoType) end

---Gets *integer* information about the current world.
---@param infoType
---| 201 # Total lines received
---| 202 # Lines received but not yet seen (new lines)
---| 203 # Total lines sent
---| 204 # Packets received
---| 205 # Packets sent
---| 206 # Total uncompressed bytes received
---| 207 # Total compresssed bytes received
---| 208 # MCCP protocol in use (0=none, 2=MCCP2)
---| 216 # Total bytes received
---| 217 # Total bytes sent
---| 218 # Count of variables
---| 219 # Count of triggers
---| 220 # Count of timers
---| 221 # Count of aliases
---| 222 # Count of queued commands
---| 224 # Count of lines in output window
---| 225 # Count of custom MXP elements
---| 226 # Count of custom MXP entities
---| 228 # World TCP/IP address (after DNS lookup, as a number)
---| 231 # Log file size (0 if no log file open)
---| 235 # Number of world windows open
---| 236 # First column of the selection in the command window (1-relative)
---| 237 # Last column of the selection in the command window (1-relative)
---| 249 # Main frame window height
---| 250 # Main frame window width
---| 259 # Status bar window height
---| 260 # Status bar window width
---| 261 # World window non-client height
---| 262 # World window non-client width
---| 263 # World window client height
---| 264 # World window client width
---| 271 # Background colour
---| 272 # Text rectangle - left (see TextRectangle function)
---| 273 # Text rectangle - top
---| 274 # Text rectangle - right
---| 275 # Text rectangle - bottom
---| 276 # Text rectangle - border offset
---| 277 # Text rectangle - border width
---| 278 # Text rectangle - outside colour
---| 279 # Text rectangle - outside style
---| 280 # Output window client height
---| 281 # Output window client width
---| 282 # Text rectangle - border colour
---| 283 # Mouse position - X (relative to world output window)
---| 284 # Mouse position - Y
---| 289 # Last line number that was terminated by an IAC/GA or IAC/EOR
---| 290 # Actual text rectangle - left
---| 291 # Actual text rectangle - top
---| 292 # Actual text rectangle - right
---| 293 # Actual text rectangle - bottom
---| 294 # Status of keyboard modifiers / mouse click
---| 296 # Current vertical position of scroll bar in first output window
---| 298 # SQLite3 version number (eg. 3007002 for version 3.7.2.)
---| 301 # Time connected
---| 304 # The current date/time
---| 306 # When this world was created/opened
---| 310 # Newlines received from the MUD (lines terminated by a newline)
---@return integer info
function GetInfo(infoType) end

---Gets *number* information about the current world.
---@param infoType
---| 212 # Output font height
---| 213 # Output font width
---| 214 # Input font height
---| 215 # Input font width
---| 232 # High-performance counter output (in seconds)
---| 240 # The average width, in pixels, of a character in the output window, in the current output font. This figure is used when calculating how many characters would fit in the current width of the output window.
---| 241 # The height, in pixels, of a character in the output window, in the current output font.
---| 243 # Font size of fixed pitch font.
---@return number info
function GetInfo(infoType) end

---Gets the connect phase of the current world.
---@param infoType
---| 227 # Connect phase
---@return ConnectPhase info
function GetInfo(infoType) end

---Gets the window state of the current world.
---@param infoType
---| 238 # Current "show state" for the world window of this world.
---@return WindowState info
function GetInfo(infoType) end

---Gets the action source of the current world.
---@param infoType
---| 238 # The "source" of the current scripted action.
---@return ActionSource info
function GetInfo(infoType) end

---Gets window position information about the current world.
---@param infoType
---| 269 # Foreground image mode
---| 270 # Background image mode
---@return miniwin.pos info
function GetInfo(infoType) end

---Gets the operating system of the current world.
---@param infoType 268
---@return operating_system|nil info
function GetInfo(infoType) end


---Gets *boolean* information about any line in the output buffer (window).
---@param lineNumber integer From 1 (the first line) to the number of lines in the output window. You can use [`GetLinesInBufferCount()`](lua://GetLinesInBufferCount) to find how many lines are currently in the output window.
---@param infoType
---| 4 # Produced by Note()
---| 5 # Produced by player input
---| 8 # Is horizontal rule (<hr>)
---@return boolean info
function GetLineInfo(lineNumber, infoType) end

---Gets *integer* information about any line in the output buffer (window).
---@param lineNumber integer From 1 (the first line) to the number of lines in the output window. You can use [`GetLinesInBufferCount()`](lua://GetLinesInBufferCount) to find how many lines are currently in the output window.
---@param infoType
---| 2 # Length of text
---| 9 # Date/time line arrived
---| 11 # Count of style runs
---| 12 # Milliseconds since high-performance timer reference point
---| 13 # Elapsed time in seconds from when the world was opened
---@return integer info
function GetLineInfo(lineNumber, infoType) end

---Gets *string* information about any line in the output buffer (window).
---@param lineNumber integer From 1 (the first line) to the number of lines in the output window. You can use [`GetLinesInBufferCount()`](lua://GetLinesInBufferCount) to find how many lines are currently in the output window.
---@param infoType
---| 1 # Text of line
---@return string info
function GetLineInfo(lineNumber, infoType) end


---Gets *boolean* information about an alias from another plugin.
---@param pluginID string ID of the plugin that contains the alias.
---@param aliasName string
---@param infoType
---| 4 # Omit from log
---| 5 # Omit from output
---| 6 # Enabled
---| 7 # Regular expression
---| 8 # Ignore case
---| 9 # Expand variables
---| 12 # Display in alias menu
---| 14 # Temporary
---| 19 # Keep evaluating
---| 21 # Echo
---| 22 # Omit from command history
---| 27 # Script is valid
---| 29 # One-shot
---@return boolean info
---
---@see GetAliasInfo - get information about an alias from the current plugin.
---@see GetPluginTimerInfo - get information about a timer from another plugin.
---@see GetPluginTriggerInfo - get information about a trigger from another plugin.
function GetPluginAliasInfo(pluginID, aliasName, infoType) end

---Gets the send-to location of an alias from another plugin.
---@param pluginID string
---@param aliasName string
---@param infoType
---| 18 # Send-to location
---@return sendto info
---
---@see GetAliasInfo - get information about an alias from the current plugin.
---@see GetPluginTimerInfo - get information about a timer from another plugin.
---@see GetPluginTriggerInfo - get information about a trigger from another plugin.
function GetPluginAliasInfo(pluginID, aliasName, infoType) end

---Gets *integer* information about an alias from another plugin.
---@param pluginID string
---@param aliasName string
---@param infoType
---| 20 # Sequence number
---| 23 # User option value
---@return integer info
---
---@see GetAliasInfo - get information about an alias from the current plugin.
---@see GetPluginTimerInfo - get information about a timer from another plugin.
---@see GetPluginTriggerInfo - get information about a trigger from another plugin.
function GetPluginAliasInfo(pluginID, aliasName, infoType) end

---Gets *string* information about an alias from another plugin.
---@param pluginID string
---@param aliasName string
---@param infoType
---| 1 # What to match on
---| 2 # What to send
---| 3 # Script procedure name
---| 16 # Group name
---| 17 # Variable name to set
---@return string info
---
---@see GetAliasInfo - get information about an alias from the current plugin.
---@see GetPluginTimerInfo - get information about a timer from another plugin.
---@see GetPluginTriggerInfo - get information about a trigger from another plugin.
function GetPluginAliasInfo(pluginID, aliasName, infoType) end


---Gets *boolean* information about a specified plugin.
---@param pluginID string
---@param infoType
---| 15 # Save state automatically
---| 16 # Scripting enabled
---| 17 # Enabled
---@return boolean info
---
---@see GetAliasInfo - get information about an alias from the current plugin.
---@see GetPluginTimerInfo - get information about a timer from another plugin.
---@see GetPluginTriggerInfo - get information about a trigger from another plugin.
function GetPluginInfo(pluginID, infoType) end

---Gets *integer* information about a specified plugin.
---@param pluginID string
---@param infoType
---| 9 # Number of triggers
---| 10 # Number of aliases
---| 11 # Number of timers
---| 12 # Number of variables
---| 13 # Date written
---| 14 # Date modified
---| 21 # Evaluation order
---| 22 # Date/time plugin was installed
---| 25 # Plugin sequence number
---@return integer info
function GetPluginInfo(pluginID, infoType) end

---Gets *number* (floating-point) information about a specified plugin.
---@param pluginID string
---@param infoType
---| 18 # Required MUSHclient version
---| 19 # Version of plugin
---@return number info
function GetPluginInfo(pluginID, infoType) end

---Gets *string* information about a specified plugin.
---@param pluginID string
---@param infoType
---| 1 # Name
---| 2 # Author
---| 3 # Description
---| 4 # Script contents
---| 5 # "lua" (script language)
---| 6 # Plugin file path
---| 7 # Unique ID
---| 8 # Purpose (short description)
---| 20 # Parent directory
---| 23 # During a CallPlugin call, the ID of the calling plugin (if any)
---@return string info
function GetPluginInfo(pluginID, infoType) end


---Gets *boolean* information about a timer from another plugin.
---@param pluginID string
---@param timerName string
---@param infoType
---| 6 # Enabled
---| 7 # One-shot
---| 8 # True to fire at a certain time, false to fire at an interval
---| 14 # Temporary
---| 15 # Send to speedwalk
---| 16 # Send to Note
---| 17 # Active when disconnected
---| 23 # Omit from output
---| 24 # Omit from log file
---| 26 # Script is valid
---@return boolean info
---
---@see GetTimerInfo - get information about a timer from the current plugin.
---@see GetPluginAliasInfo - get information about an alias from another plugin.
---@see GetPluginTriggerInfo - get information about a trigger from another plugin.
function GetPluginTimerInfo(pluginID, timerName, infoType) end

---Gets the send-to location of a timer from another plugin.
---@param pluginID string
---@param timerName string
---@param infoType
---| 20 # Send-to location
---@return sendto info
---
---@see GetTimerInfo - get information about a timer from the current plugin.
---@see GetPluginAliasInfo - get information about an alias from another plugin.
---@see GetPluginTriggerInfo - get information about a trigger from another plugin.
function GetPluginTimerInfo(pluginID, timerName, infoType) end

---Gets *integer* information about a timer from another plugin.
---@param pluginID string
---@param timerName string
---@param infoType
---| 1 # The hour
---| 2 # The minute
---| 3 # The second
---| 11 # Date/time timer last fired
---| 12 # Date/time timer will fire next
---| 13 # Number of seconds until timer will fire next
---| 21 # User option value
---@return integer info
---
---@see GetTimerInfo - get information about a timer from the current plugin.
---@see GetPluginAliasInfo - get information about an alias from another plugin.
---@see GetPluginTriggerInfo - get information about a trigger from another plugin.
function GetPluginTimerInfo(pluginID, timerName, infoType) end

---Gets *string* information about a timer from another plugin.
---@param pluginID string
---@param timerName string
---@param infoType
---| 4 # What to send
---| 5 # Script procedure name
---| 19 # Group name
---| 22 # Timer label
---@return string info
---
---@see GetTimerInfo - get information about a timer from the current plugin.
---@see GetPluginAliasInfo - get information about an alias from another plugin.
---@see GetPluginTriggerInfo - get information about a trigger from another plugin.
function GetPluginTimerInfo(pluginID, timerName, infoType) end


---Gets *boolean* information about a trigger from another plugin.
---@param pluginID string
---@param triggerName string
---@param infoType
---| 5 # Omit from log
---| 6 # Omit from output
---| 7 # Keep evaluating
---| 8 # Enabled
---| 9 # Regular expression
---| 10 # Ignore case
---| 11 # Repeat on same line
---| 12 # Play sound if inactive
---| 13 # Expand variables
---| 23 # Temporary
---| 25 # Make wildcards lower case
---| 34 # Script is valid
---| 36 # One-shot
---@return boolean info
---
---@see GetTriggerInfo - get information about a trigger from the current plugin.
---@see GetPluginAliasInfo - get information about an alias from another plugin.
---@see GetPluginTimerInfo - get information about a timer from another plugin.
function GetPluginTriggerInfo(pluginID, triggerName, infoType) end

---Gets the send-to location of a trigger from another plugin.
---@param pluginID string
---@param triggerName string
---@param infoType
---| 15 # Send-to location
---@return sendto info
---
---@see GetTriggerInfo - get information about a trigger from the current plugin.
---@see GetPluginAliasInfo - get information about an alias from another plugin.
---@see GetPluginTimerInfo - get information about a timer from another plugin.
function GetPluginTriggerInfo(pluginID, triggerName, infoType) end

---Gets *integer* information about a trigger from another plugin.
---@param pluginID string
---@param triggerName string
---@param infoType
---| 14 # Which wildcard to send to clipboard
---| 16 # Sequence
---| 18 # Change to style
---| 28 # User option value
---| 29 # Foreground BBGGRR colour code
---| 30 # Background BBGGRR colour code
---@return integer info
---
---@see GetTriggerInfo - get information about a trigger from the current plugin.
---@see GetPluginAliasInfo - get information about an alias from another plugin.
---@see GetPluginTimerInfo - get information about a timer from another plugin.
function GetPluginTriggerInfo(pluginID, triggerName, infoType) end

---Gets *string* information about a trigger from another plugin.
---@param pluginID string
---@param triggerName string
---@param infoType
---| 1 # What to match on
---| 2 # What to send
---| 3 # Sound to play
---| 4 # Script procedure name
---| 26 # Group name
---| 27 # Variable name
---@return string info
---
---@see GetTriggerInfo - get information about a trigger from the current plugin.
---@see GetPluginAliasInfo - get information about an alias from another plugin.
---@see GetPluginTimerInfo - get information about a timer from another plugin.
function GetPluginTriggerInfo(pluginID, triggerName, infoType) end


---Gets *boolean* information about a specified style run for a specified line in the output window.
---@param lineNumber integer -- From 1 (the first line) to the number of lines in the output window. You can use [`GetLinesInBufferCount()`](lua://GetLinesInBufferCount) to find how many lines are currently in the output window.
---@param styleNumber integer -- From 1 (the first style on the line) to the number of styles in that line. You can use [`GetLineInfo(lineNumber, 11)`](lua://GetLineInfo) to find the number of styles in each line.
---@param infoType
---| 8 # Bold
---| 9 # Underlined
---| 10 # Blinking
---| 11 # Inverse
---@return boolean info
function GetStyleInfo(lineNumber, styleNumber, infoType) end

---Gets the action type of a specified style run for a specified line in the output window.
---@param lineNumber integer -- From 1 (the first line) to the number of lines in the output window. You can use [`GetLinesInBufferCount()`](lua://GetLinesInBufferCount) to find how many lines are currently in the output window.
---@param styleNumber integer -- From 1 (the first style on the line) to the number of styles in that line. You can use [`GetLineInfo(lineNumber, 11)`](lua://GetLineInfo) to find the number of styles in each line.
---@param infoType
---| 4 # Action type
---@return LineAction info
function GetStyleInfo(lineNumber, styleNumber, infoType) end

---Gets *integer* information about a specified style run for a specified line in the output window.
---@param lineNumber integer -- From 1 (the first line) to the number of lines in the output window. You can use [`GetLinesInBufferCount()`](lua://GetLinesInBufferCount) to find how many lines are currently in the output window.
---@param styleNumber integer -- From 1 (the first style on the line) to the number of styles in that line. You can use [`GetLineInfo(lineNumber, 11)`](lua://GetLineInfo) to find the number of styles in each line.
---@param infoType
---| 2 # Length of style run
---| 3 # Starting column of style
---| 14 # Foreground text colour in BBGGRR
---| 15 # Background colour in BBGGRR
---@return integer info
function GetStyleInfo(lineNumber, styleNumber, infoType) end

---Gets *string* information about a specified style run for a specified line in the output window.
---@param lineNumber integer -- From 1 (the first line) to the number of lines in the output window. You can use [`GetLinesInBufferCount()`](lua://GetLinesInBufferCount) to find how many lines are currently in the output window.
---@param styleNumber integer -- From 1 (the first style on the line) to the number of styles in that line. You can use [`GetLineInfo(lineNumber, 11)`](lua://GetLineInfo) to find the number of styles in each line.
---@param infoType
---| 1 # Text of style
---| 5 # Action (e.g. what to send)
---| 6 # Hint (what to show)
---@return string info
function GetStyleInfo(lineNumber, styleNumber, infoType) end


---Gets *boolean* information about a timer.
---@param timerName string
---@param infoType
---| 6 # Enabled
---| 7 # One-shot
---| 8 # True to fire at a certain time, false to fire at an interval
---| 14 # Temporary
---| 15 # Send to speedwalk
---| 16 # Send to Note
---| 17 # Active when disconnected
---| 23 # Omit from output
---| 24 # Omit from log file
---| 26 # Script is valid
---@return boolean info
---
---@see GetPluginTimerInfo - get information about a timer from another plugin.
---@see GetAliasInfo - get information about an alias.
---@see GetTriggerInfo - get information about a trigger.
function GetTimerInfo(timerName, infoType) end

---Gets the send-to location of a timer.
---@param timerName string
---@param infoType
---| 20 # Send-to location
---@return sendto info
---
---@see GetPluginTimerInfo - get information about a timer from another plugin.
---@see GetAliasInfo - get information about an alias.
---@see GetTriggerInfo - get information about a trigger.
function GetTimerInfo(timerName, infoType) end

---Gets *integer* information about a timer.
---@param timerName string
---@param infoType
---| 1 # The hour
---| 2 # The minute
---| 3 # The second
---| 11 # Date/time timer last fired
---| 12 # Date/time timer will fire next
---| 13 # Number of seconds until timer will fire next
---| 21 # User option value
---@return integer info
---
---@see GetPluginTimerInfo - get information about a timer from another plugin.
---@see GetAliasInfo - get information about an alias.
---@see GetTriggerInfo - get information about a trigger.
function GetTimerInfo(timerName, infoType) end

---Gets *string* information about a timer.
---@param timerName string
---@param infoType
---| 4 # What to send
---| 5 # Script procedure name
---| 19 # Group name
---| 22 # Timer label
---@return string info
---
---@see GetPluginTimerInfo - get information about a timer from another plugin.
---@see GetAliasInfo - get information about an alias.
---@see GetTriggerInfo - get information about a trigger.
function GetTimerInfo(timerName, infoType) end


---Gets *boolean* information about a trigger.
---@param triggerName string
---@param infoType
---| 5 # Omit from log
---| 6 # Omit from output
---| 7 # Keep evaluating
---| 8 # Enabled
---| 9 # Regular expression
---| 10 # Ignore case
---| 11 # Repeat on same line
---| 12 # Play sound if inactive
---| 13 # Expand variables
---| 23 # Temporary
---| 25 # Make wildcards lower case
---| 34 # Script is valid
---| 36 # One-shot
---@return boolean info
---
---@see GetPluginTriggerInfo - get information about a trigger from another plugin.
---@see GetAliasInfo - get information about an alias.
---@see GetTimerInfo - get information about a timer.
function GetTriggerInfo(triggerName, infoType) end

---Gets the send-to location of a trigger.
---@param triggerName string
---@param infoType
---| 15 # Send-to location
---@return sendto info
---
---@see GetPluginTriggerInfo - get information about a trigger from another plugin.
---@see GetAliasInfo - get information about an alias.
---@see GetTimerInfo - get information about a timer.
function GetTriggerInfo(triggerName, infoType) end

---Gets *integer* information about a trigger.
---@param triggerName string
---@param infoType
---| 14 # Which wildcard to send to clipboard
---| 16 # Sequence
---| 18 # Change to style
---| 28 # User option value
---| 29 # Foreground BBGGRR colour code
---| 30 # Background BBGGRR colour code
---@return integer info
---
---@see GetPluginTriggerInfo - get information about a trigger from another plugin.
---@see GetAliasInfo - get information about an alias.
---@see GetTimerInfo - get information about a timer.
function GetTriggerInfo(triggerName, infoType) end

---Gets *string* information about a trigger.
---@param triggerName string
---@param infoType
---| 1 # What to match on
---| 2 # What to send
---| 3 # Sound to play
---| 4 # Script procedure name
---| 26 # Group name
---| 27 # Variable name
---@return string info
---
---@see GetPluginTriggerInfo - get information about a trigger from another plugin.
---@see GetAliasInfo - get information about an alias.
---@see GetTimerInfo - get information about a timer.
function GetTriggerInfo(triggerName, infoType) end


---Gets the corresponding MUSHclient version of this API.
---@return string version
function Version() end


---Gets *boolean* information about a loaded font.
---@param windowName string
---@param fontID string
---@param infoType
---| 16 # Italic
---| 17 # Underlined
---| 18 # Struck Out
---@return boolean info
---
---@see WindowHotspotInfo - get information about a miniwindow hotspot.
---@see WindowImageInfo - get information about a miniwindow image.
---@see WindowInfo - get information about a miniwindow.
function WindowFontInfo(windowName, fontID, infoType) end

---Gets *integer* information about a loaded font.
---@param windowName string
---@param fontID string
---@param infoType
---| 1 # Height - Specifies the height (ascent + descent) of characters.
---@return integer info
---
---@see WindowHotspotInfo - get information about a miniwindow hotspot.
---@see WindowImageInfo - get information about a miniwindow image.
---@see WindowInfo - get information about a miniwindow.
function WindowFontInfo(windowName, fontID, infoType) end

---Gets *number* information about a loaded font.
---@param windowName string
---@param fontID string
---@param infoType
---| 1 # Height - Specifies the height (ascent + descent) of characters.
---| 2 # Ascent - Specifies the ascent (units above the base line) of characters.
---| 3 # Descent - Specifies the descent (units below the base line) of characters.
---| 4 # Leading - Specifies the amount of leading (space) inside the bounds set by the height. Accent marks and other diacritical characters may occur in this area.
---| 6 # Average character width.
---| 7 # Maximum character width.
---| 9 # Overhang - The right-ward distance of the left-most pixel of the character from the logical origin of the character. This value is negative if the pixels of the character extend to the left of the logical origin.
---| 19 # Pitch And Family - Specifies information about the pitch, the technology, and the family of a physical font.
---@return number info
---
---@see WindowHotspotInfo - get information about a miniwindow hotspot.
---@see WindowImageInfo - get information about a miniwindow image.
---@see WindowInfo - get information about a miniwindow.
function WindowFontInfo(windowName, fontID, infoType) end

---Gets *string* information about a loaded font.
---@param windowName string
---@param fontID string
---@param infoType
---| 14 # Default Character - Specifies the value of the character to be substituted for characters not in the font. This is always "�".
---| 15 # Break Character - Specifies the value of the character that will be used to define word breaks for text justification. This is always " ".
---| 21 # Name - Specifies the name of the font.
---@return string info
---
---@see WindowHotspotInfo - get information about a miniwindow hotspot.
---@see WindowImageInfo - get information about a miniwindow image.
---@see WindowInfo - get information about a miniwindow.
function WindowFontInfo(windowName, fontID, infoType) end


---Gets the mouseover cursor of a hotspot.
---@param windowName string
---@param hotspotID string
---@param infoType
---| 11 # Cursor
---@return miniwin.cursor info See [`miniwin.cursor`](lua://miniwin.cursor).
---
---@see WindowFontInfo - get information about a miniwindow font.
---@see WindowImageInfo - get information about a miniwindow image.
---@see WindowInfo - get information about a miniwindow.
function WindowHotspotInfo(windowName, hotspotID, infoType) end

---Gets *integer* information about a hotspot.
---@param windowName string
---@param hotspotID string
---@param infoType
---| 1 # Rectangle: Left
---| 2 # Rectangle: Top
---| 3 # Rectangle: Right
---| 4 # Rectangle: Bottom
---| 12 # Flags
---| 15 # DragHandler flags
---@return integer info
---
---@see WindowFontInfo - get information about a miniwindow font.
---@see WindowImageInfo - get information about a miniwindow image.
---@see WindowInfo - get information about a miniwindow.
function WindowHotspotInfo(windowName, hotspotID, infoType) end

---Gets *string* information about a hotspot.
---@param windowName string
---@param hotspotID string
---@param infoType
---| 5 # MouseOver function name
---| 6 # CancelMouseOver function name
---| 7 # MouseDown function name
---| 8 # CancelMouseDown function name
---| 9 # MouseUp function name
---| 10 # Tooltip text
---| 13 # MoveCallback function name
---| 14 # ReleaseCallback function name
---@return string info
---
---@see WindowFontInfo - get information about a miniwindow font.
---@see WindowImageInfo - get information about a miniwindow image.
---@see WindowInfo - get information about a miniwindow.
function WindowHotspotInfo(windowName, hotspotID, infoType) end


---Gets *integer* information about a loaded image.
---@param windowName string
---@param imageID string
---@param infoType
---| 2 # Width, in pixels, of the bitmap.
---| 3 # Height, in pixels, of the bitmap.
---| 4 # WidthBytes - Specifies the number of bytes in each scan line.
---| 5 # Planes - Specifies the count of colour planes.
---| 6 # BitsPixel - Specifies the number of bits required to indicate the colour of a pixel.
---@return integer info
---
---@see WindowFontInfo - get information about a miniwindow font.
---@see WindowHotspotInfo - get information about a miniwindow hotspot.
---@see WindowInfo - get information about a miniwindow.
function WindowImageInfo(windowName, imageID, infoType) end


---Gets *boolean* information about a miniwindow.
---@param windowName string
---@param infoType
---| 5 # Visible
---| 6 # Hidden
---@return boolean info
---
---@see WindowFontInfo - get information about a miniwindow font.
---@see WindowHotspotInfo - get information about a miniwindow hotspot.
---@see WindowImageInfo - get information about a miniwindow image.
function WindowInfo(windowName, infoType) end

---Gets *integer* information about a miniwindow.
---@param windowName string
---@param infoType
---| 1 # Left (from WindowCreate)
---| 2 # Top (from WindowCreate)
---| 3 # Width
---| 4 # Height
---| 8 # Flags
---| 9 # Background BBGGRR colour code
---| 10 # Current left
---| 11 # Current top
---| 12 # Current right
---| 13 # Current bottom
---| 14 # Where mouse last was - X position - relative to miniwindow
---| 15 # Where mouse last was - Y position - relative to miniwindow
---| 17 # Where mouse last was - X position - relative to output window
---| 18 # Where mouse last was - Y position - relative to output window
---| 21 # Date/time the miniwindow was installed.
---| 22 # Z-Order of the miniwindow.
---| 23 # Plugin ID of the plugin that created this miniwindow (empty if none).
---@return integer info
---
---@see WindowFontInfo - get information about a miniwindow font.
---@see WindowHotspotInfo - get information about a miniwindow hotspot.
---@see WindowImageInfo - get information about a miniwindow image.
function WindowInfo(windowName, infoType) end

---Gets *string* information about a miniwindow.
---@param windowName string
---@param infoType
---| 23 # Plugin ID of the plugin that created this miniwindow (empty if none).
---@return string info
---
---@see WindowFontInfo - get information about a miniwindow font.
---@see WindowHotspotInfo - get information about a miniwindow hotspot.
---@see WindowImageInfo - get information about a miniwindow image.
function WindowInfo(windowName, infoType) end

---Gets the position of a miniwindow.
---@param windowName string
---@param infoType
---| 7 # Layout position code
---@return miniwin.pos info See [`miniwin.pos`](lua://miniwin.pos).
---
---@see WindowFontInfo - get information about a miniwindow font.
---@see WindowHotspotInfo - get information about a miniwindow hotspot.
---@see WindowImageInfo - get information about a miniwindow image.
function WindowInfo(windowName, infoType) end


---This returns the name of the current world. Equivalent to [`GetAlphaOption("name")`](lua://GetAlphaOption).
---@return string name World name.
function WorldName() end
