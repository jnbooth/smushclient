---@meta

json = {}

---@param obj any
---@return string
function json.encode(obj) end

---@param encoded string
---@return any
function json.decode(encoded) end

rex = {}

---@param pattern string
---@return any
function rex.new(pattern) end

sqlite3 = {}

---@param filePath string
---@return any
function sqlite3.open(filePath) end

utils = {}

---@param encoded string
---@return string
function utils.base64decode(encoded) end

---@param text string
---@return string
function utils.base64encode(text) end

---@param fontType? 0 | 1 | 2 | 3
---@return string, integer
function utils.getsystemfont(fontType) end

---@type string | nil
SMUSHCLIENT_VERSION = nil

custom_colour = {
  NoChange = -1,
  Custom1 = 0x8080FF,
  Custom2 = 0x80FFFF,
  Custom3 = 0x80FF80,
  Custom4 = 0xFFFF80,
  Custom5 = 0xFF8000,
  Custom6 = 0xC080FF,
  Custom7 = 0x0000FF,
  Custom8 = 0xC08000,
  Custom9 = 0xFF00FF,
  Custom10 = 0x404080,
  Custom11 = 0x4080FF,
  Custom12 = 0x808000,
  Custom13 = 0x804000,
  Custom14 = 0x8000FF,
  Custom15 = 0x008000,
  Custom16 = 0xFF0000,
  CustomOther = -1,
}

---@enum ApiCode
error_code = {  eOK = 0,                             ---No error
  eWorldOpen = 30001,                  ---The world is already open
  eWorldClosed = 30002,                ---The world is closed, this action cannot be performed
  eNoNameSpecified = 30003,            ---No name has been specified where one is required
  eCannotPlaySound = 30004,            ---The sound file could not be played
  eTriggerNotFound = 30005,            ---The specified trigger name does not exist
  eTriggerAlreadyExists = 30006,       ---Attempt to add a trigger that already exists
  eTriggerCannotBeEmpty = 30007,       ---The trigger "match" string cannot be empty
  eInvalidObjectLabel = 30008,         ---The name of this object is invalid
  eScriptNameNotLocated = 30009,       ---Script name is not in the script file
  eAliasNotFound = 30010,              ---The specified alias name does not exist
  eAliasAlreadyExists = 30011,         ---Attempt to add a alias that already exists
  eAliasCannotBeEmpty = 30012,         ---The alias "match" string cannot be empty
  eCouldNotOpenFile = 30013,           ---Unable to open requested file
  eLogFileNotOpen = 30014,             ---Log file was not open
  eLogFileAlreadyOpen = 30015,         ---Log file was already open
  eLogFileBadWrite = 30016,            ---Bad write to log file
  eTimerNotFound = 30017,              ---The specified timer name does not exist
  eTimerAlreadyExists = 30018,         ---Attempt to add a timer that already exists
  eVariableNotFound = 30019,           ---Attempt to delete a variable that does not exist
  eCommandNotEmpty = 30020,            ---Attempt to use SetCommand with a non-empty command window
  eBadRegularExpression = 30021,       ---Bad regular expression syntax
  eTimeInvalid = 30022,                ---Time given to AddTimer is invalid
  eBadMapItem = 30023,                 ---Direction given to AddToMapper is invalid
  eNoMapItems = 30024,                 ---No items in mapper
  eUnknownOption = 30025,              ---Option name not found
  eOptionOutOfRange = 30026,           ---New value for option is out of range
  eTriggerSequenceOutOfRange = 30027,  ---Trigger sequence value invalid
  eTriggerSendToInvalid = 30028,       ---Where to send trigger text to is invalid
  eTriggerLabelNotSpecified = 30029,   ---Trigger label not specified/invalid for 'send to variable'
  ePluginFileNotFound = 30030,         ---File name specified for plugin not found
  eProblemsLoadingPlugin = 30031,      ---There was a parsing or other problem loading the plugin
  ePluginCannotSetOption = 30032,      ---Plugin is not allowed to set this option
  ePluginCannotGetOption = 30033,      ---Plugin is not allowed to get this option
  eNoSuchPlugin = 30034,               ---Requested plugin is not installed
  eNotAPlugin = 30035,                 ---Only a plugin can do this
  eNoSuchRoutine = 30036,              ---Plugin does not support that subroutine (subroutine not in script)
  ePluginDoesNotSaveState = 30037,     ---Plugin does not support saving state
  ePluginCouldNotSaveState = 30037,    ---Plugin could not save state (eg. no state directory)
  ePluginDisabled = 30039,             ---Plugin is currently disabled
  eErrorCallingPluginRoutine = 30040,  ---Could not call plugin routine
  eCommandsNestedTooDeeply = 30041,    ---Calls to "Execute" nested too deeply
  eCannotCreateChatSocket = 30042,     ---Unable to create socket for chat connection
  eCannotLookupDomainName = 30043,     ---Unable to do DNS (domain name) lookup for chat connection
  eNoChatConnections = 30044,          ---No chat connections open
  eChatPersonNotFound = 30045,         ---Requested chat person not connected
  eBadParameter = 30046,               ---General problem with a parameter to a script call
  eChatAlreadyListening = 30047,       ---Already listening for incoming chats
  eChatIDNotFound = 30048,             ---Chat session with that ID not found
  eChatAlreadyConnected = 30049,       ---Already connected to that server/port
  eClipboardEmpty = 30050,             ---Cannot get (text from the) clipboard
  eFileNotFound = 30051,               ---Cannot open the specified file
  eAlreadyTransferringFile = 30052,    ---Already transferring a file
  eNotTransferringFile = 30053,        ---Not transferring a file
  eNoSuchCommand = 30054,              ---There is not a command of that name
  eArrayAlreadyExists = 30055,         ---That array already exists
  eBadKeyName = 30056,                 ---That name is not permitted for a key
  eArrayDoesNotExist = 30056,          ---That array does not exist
  eArrayNotEvenNumberOfValues = 30057, ---Values to be imported into array are not in pairs
  eImportedWithDuplicates = 30058,     ---Import succeeded, however some values were overwritten
  eBadDelimiter = 30059,               ---Import/export delimiter must be a single character, other than backslash
  eSetReplacingExistingValue = 30060,  ---Array element set, existing value overwritten
  eKeyDoesNotExist = 30061,            ---Array key does not exist
  eCannotImport = 30062,               ---Cannot import because cannot find unused temporary character
  eItemInUse = 30063,                  ---Cannot delete trigger/alias/timer because it is executing a script
  eSpellCheckNotActive = 30064,        ---Spell checker is not active
  eCannotAddFont = 30065,              ---Cannot create requested font
  ePenStyleNotValid = 30066,           ---Invalid settings for pen parameter
  eUnableToLoadImage = 30067,          ---Bitmap image could not be loaded
  eImageNotInstalled = 30068,          ---Image has not been loaded into window
  eInvalidNumberOfPoints = 30069,      ---Number of points supplied is incorrect
  eInvalidPoint = 30070,               ---Point is not numeric
  eHotspotPluginChanged = 30071,       ---Hotspot processing must all be in same plugin
  eHotspotNotInstalled = 30072,        ---Hotspot has not been defined for this window
  eNoSuchWindow = 30073,               ---Requested miniwindow does not exist
  eBrushStyleNotValid = 30074,         ---Invalid settings for brush parameter
}

---@type table<ApiCode, string>
error_desc = {}

miniwin = {
  pos_stretch_to_view = 0,
  pos_stretch_to_view_with_aspect = 1,
  pos_stretch_to_owner = 2,
  pos_stretch_to_owner_with_aspect = 3,
  pos_top_left = 4,
  pos_top_center = 5,
  pos_top_right = 6,
  pos_center_right = 7,
  pos_bottom_right = 8,
  pos_bottom_center = 9,
  pos_bottom_left = 10,
  pos_center_left = 11,
  pos_center_all = 12,
  pos_tile = 13,

  create_underneath = 1,
  create_absolute_location = 2,
  create_transparent = 4,
  create_ignore_mouse = 8,
  create_keep_hotspots = 16,

  pen_solid = 0,
  pen_dash = 1,
  pen_dot = 2,
  pen_dash_dot = 3,
  pen_dash_dot_dot = 4,
  pen_null = 5,
  pen_inside_frame = 6,

  pen_endcap_round = 0,
  pen_endcap_square = 256,
  pen_endcap_flat = 512,

  pen_join_round = 0,
  pen_join_bevel = 4096,
  pen_join_miter = 8192,

  brush_solid = 0,
  brush_null = 1,
  brush_hatch_horizontal = 2,
  brush_hatch_vertical = 3,
  brush_hatch_forwards_diagonal = 4,
  brush_hatch_backwards_diagonal = 5,
  brush_hatch_cross = 6,
  brush_hatch_cross_diagonal = 7,
  brush_fine_pattern = 8,
  brush_medium_pattern = 9,
  brush_coarse_pattern = 10,
  brush_waves_horizontal = 11,
  brush_waves_vertical = 12,

  rect_frame = 1,
  rect_fill = 2,
  rect_invert = 3,
  rect_3d_rect = 4,
  rect_draw_edge = 5,
  rect_flood_fill_border = 6,
  rect_flood_fill_surface = 7,
  rect_edge_raised = 5,
  rect_edge_etched = 6,
  rect_edge_bump = 9,
  rect_edge_sunken = 10,
  rect_edge_at_top_left = 3,
  rect_edge_at_top_right = 6,
  rect_edge_at_bottom_left = 9,
  rect_edge_at_bottom_right = 12,
  rect_edge_at_all = 15,
  rect_diagonal_end_top_left = 19,
  rect_diagonal_end_top_right = 22,
  rect_diagonal_end_bottom_left = 25,
  rect_diagonal_end_bottom_right = 28,
  rect_option_fill_middle = 0x0800,
  rect_option_softer_buttons = 0x1000,
  rect_option_flat_borders = 0x4000,
  rect_option_monochrom_borders = 0x8000,

  circle_ellipse = 1,
  circle_rectangle = 2,
  circle_round_rectangle = 3,
  circle_chord = 4,
  circle_pie = 5,

  gradient_horizontal = 1,
  gradient_vertical = 2,
  gradient_texture = 3,

  font_charset_ansi = 0,
  font_charset_default = 1,
  font_charset_symbol = 2,

  font_family_any = 0,
  font_family_roman = 16,
  font_family_swiss = 32,
  font_family_modern = 48,
  font_family_script = 64,
  font_family_decorative = 80,

  font_pitch_default = 0,
  font_pitch_fixed = 1,
  font_pitch_variable = 2,
  font_pitch_monospaced = 8,

  font_truetype = 4,

  image_copy = 1,
  image_stretch = 2,
  image_transparent_copy = 3,

  image_fill_ellipse = 1,
  image_fill_rectangle = 2,
  image_fill_round_fill_rectangle = 3,

  filter_noise = 1,
  filter_monochrome_noise = 2,
  filter_blur = 3,
  filter_sharpen = 4,
  filter_find_edges = 5,
  filter_emboss = 6,
  filter_brightness = 7,
  filter_contrast = 8,
  filter_gamma = 9,
  filter_red_brightness = 10,
  filter_red_contrast = 11,
  filter_red_gamma = 12,
  filter_green_brightness = 13,
  filter_green_contrast = 14,
  filter_green_gamma = 15,
  filter_blue_brightness = 16,
  filter_blue_contrast = 17,
  filter_blue_gamma = 18,
  filter_grayscale = 19,
  filter_normal_grayscale = 20,
  filter_brightness_multiply = 21,
  filter_red_brightness_multiply = 22,
  filter_green_brightness_multiply = 23,
  filter_blue_brightness_multiply = 24,
  filter_lesser_blur = 25,
  filter_minor_blur = 26,
  filter_average = 27,

  blend_normal = 1,
  blend_average = 2,
  blend_interpolate = 3,
  blend_dissolve = 4,
  blend_darken = 5,
  blend_multiply = 6,
  blend_colour_burn = 7,
  blend_linear_burn = 8,
  blend_inverse_colour_burn = 9,
  blend_subtract = 10,
  blend_lighten = 11,
  blend_screen = 12,
  blend_colour_dodge = 13,
  blend_linear_dodge = 14,
  blend_inverse_colour_dodge = 15,
  blend_add = 16,
  blend_overlay = 17,
  blend_soft_light = 18,
  blend_hard_light = 19,
  blend_vivid_light = 20,
  blend_linear_light = 21,
  blend_pin_light = 22,
  blend_hard_mix = 23,
  blend_difference = 24,
  blend_exclusion = 25,
  blend_reflect = 26,
  blend_glow = 27,
  blend_freeze = 28,
  blend_heat = 29,
  blend_negation = 30,
  blend_phoenix = 31,
  blend_stamp = 32,
  blend_xor = 33,
  blend_and = 34,
  blend_or = 35,
  blend_red = 36,
  blend_green = 37,
  blend_blue = 38,
  blend_yellow = 39,
  blend_cyan = 40,
  blend_magenta = 41,
  blend_green_limited_by_red = 42,
  blend_green_limited_by_blue = 43,
  blend_green_limited_by_average_of_red_and_blue = 44,
  blend_blue_limited_by_red = 45,
  blend_blue_limited_by_green = 46,
  blend_blue_limited_by_average_of_red_and_green = 47,
  blend_red_limited_by_green = 48,
  blend_red_limited_by_blue = 49,
  blend_red_limited_by_average_of_green_and_blue = 50,
  blend_red_only = 51,
  blend_green_only = 52,
  blend_blue_only = 53,
  blend_discard_red = 54,
  blend_discard_green = 55,
  blend_discard_blue = 56,
  blend_all_red = 57,
  blend_all_green = 58,
  blend_all_blue = 59,
  blend_hue_mode = 60,
  blend_saturation_mode = 61,
  blend_colour_mode = 62,
  blend_luminance_mode = 63,
  blend_hsl = 64,

  cursor_none = -1,
  cursor_arrow = 0,
  cursor_hand = 1,
  cursor_ibeam = 2,
  cursor_plus = 3,
  cursor_wait = 4,
  cursor_up = 5,
  cursor_nw_se_arrow = 6,
  cursor_ne_sw_arrow = 7,
  cursor_ew_arrow = 8,
  cursor_ns_arrow = 9,
  cursor_both_arrow = 10,
  cursor_x = 11,
  cursor_help = 12,

  hotspot_report_all_mouseovers = 1,

  hotspot_got_shift = 1,
  hotspot_got_control = 2,
  hotspot_got_alt = 4,
  hotspot_got_lh_mouse = 16,
  hotspot_got_rh_mouse = 32,
  hotspot_got_dbl_click = 64,
  hotspot_got_not_first = 128,
  hotspot_got_middle_mouse = 512,

  merge_straight = 0,
  merge_transparent = 1,

  drag_got_shift = 1,
  drag_got_control = 2,
  drag_got_alt = 4,

  wheel_got_shift = 1,
  wheel_got_control = 2,
  wheel_got_alt = 4,
  wheel_scroll_back = 256
}

---@enum OperatingSystem
operating_system = {
  Windows = 2,
  MacOS = 100,
  Linux = 200,
}

---@enum SendTo
sendto = {
  world = 0,
  command = 1,
  output = 2,
  status = 3,
  notepad = 4,
  notepadappend = 5,
  logfile = 6,
  notepadreplace = 7,
  commandqueue = 8,
  variable = 9,
  execute = 10,
  speedwalk = 11,
  script = 12,
  immediate = 13,
  scriptafteromit = 14,
}

---@enum SQLite3Code
sqlite3 = {
  OK = 0,          ---Successful result
  ERROR = 1,       ---Generic error
  INTERNAL = 2,    ---Internal logic error in SQLite
  PERM = 3,        ---Access permission denied
  ABORT = 4,       ---Callback routine requested an abort
  BUSY = 5,        ---The database file is locked
  LOCKED = 6,      ---A table in the database is locked
  NOMEM = 7,       ---A `malloc()` failed
  READONLY = 8,    ---Attempt to write a readonly database
  INTERRUPT = 9,   ---Operation terminated by `sqlite3_interrupt()`
  IOERR = 10,      ---Some kind of disk I/O error occurred
  CORRUPT = 11,    ---The database disk image is malformed
  NOTFOUND = 12,   ---Unknown opcode in `sqlite_file_control()`
  FULL = 13,       ---Insertion failed because database is full
  CANTOPEN = 14,   ---Unable to open the database file
  PROTOCOL = 15,   ---Database lock protocol error
  EMPTY = 16,      ---Internal use only
  SCHEMA = 17,     ---The database schema changed
  TOOBIG = 18,     ---String or BLOB exceeds size limit
  CONSTRAINT = 19, ---Abort due to constraint violation
  MISMATCH = 20,   ---Data type mismatch
  MISUSE = 21,     ---Library used incorrectly
  NOLFS = 22,      ---Uses OS features not supported on host
  AUTH = 23,       ---Authorization denied
  FORMAT = 24,     ---Not used
  RANGE = 25,      ---2nd parameter to sqlite3_bind out of range
  NOTADB = 26,     ---File opened that is not a database file
  NOTICE = 27,     ---Notifications from `sqlite3_log()`
  WARNING = 28,    ---Warnings from `sqlite3_log()`
  ROW = 100,       ---`sqlite3_step()` has another row ready
  DONE = 101,      ---`sqlite3_step()` has finished executing
  INTEGER = 1,
  FLOAT = 2,
  BLOB = 4,
  NULL = 5,
  TEXT = 3
}

---@enum AliasFlag
alias_flag = {
  Enabled = 1,             ---Enable
  KeepEvaluating = 8,      ---Keep evaluating
  IgnoreAliasCase = 32,    ---Ignore case when matching
  OmitFromLogFile = 64,    ---Omit from log file
  RegularExpression = 128, ---Uses regular expression
  ExpandVariables = 512,   ---Expand variables like @direction
  Replace = 1024,          ---Replace existing of same name
  AliasSpeedWalk = 2048,   ---Interpret send string as a speed walk string
  AliasQueue = 4096,       ---Queue this alias for sending at the speedwalking delay interval
  AliasMenu = 8192,        ---This alias appears on the alias menu
  Temporary = 16384,       ---Temporary - do not save to world file
  OneShot = 32768,         ---If set, only fires once
}

---@enum TimerFlag
timer_flag = {
  Enabled = 1,           ---Enable
  AtTime = 2,            ---If not set, time is "every"
  OneShot = 4,           ---If set, only fires once
  TimerSpeedWalk = 8,    ---Timer does a speed walk when it fires
  TimerNote = 16,        ---Timer does a world.Note when it fires
  ActiveWhenClosed = 32, ---Timer fires even when world is disconnected
  Replace = 1024,        ---Replace existing of same name
  Temporary = 16384,     ---Temporary - do not save to world file
}

---@enum TriggerFlag
trigger_flag = {
  Enabled = 1,              ---Enable
  OmitFromLog = 2,          ---Omit from log file
  OmitFromOutput = 4,       ---Omit from output
  KeepEvaluating = 8,       ---Keep evaluating
  IgnoreCase = 16,          ---Ignore case when matching
  RegularExpression = 32,   ---Uses regular expression
  ExpandVariables = 512,    ---Expand variables like @direction
  Replace = 1024,           ---Replace existing of same name
  LowercaseWildcard = 2048, ---Wildcards forced to lower-case
  Temporary = 16384,        ---Temporary - do not save to world file
  OneShot = 32768,          ---If set, only fires once
}

---@alias ColourParam
---| integer # BBGGRR colour code
---| string # Colour name or hex code

---@alias LinkAction
---| 0 # No action
---| 1 # Command: send to world
---| 2 # Hyperlink: open in browser
---| 3 # Prompt: send to input field

---@alias WindowBrushStyle
---| 0 # Solid
---| 1 # No fill
---| 2 # Hatch: horizontal
---| 3 # Hatch: vertical
---| 4 # Hatch: forwards diagonal
---| 5 # Hatch: backwards diagonal
---| 6 # Hatch: cross
---| 7 # Hatch: diagonal cross
---| 8 # Fine pattern
---| 9 # Medium pattern
---| 10 # Coarse pattern
---| 11 # Hatch: horizontal (same as 2)
---| 12 # Hatch: vertical (same as 3)

---@alias WindowButtonFrame
---| 5 # Raised
---| 6 # Etched
---| 9 # Bump
---| 10 # Sunken

---@alias WindowCursorShape
---| -1 # no cursor
---| 0 # arrow
---| 1 # hand
---| 2 # I-beam
---| 3 # + symbol
---| 4 # wait (hour-glass)
---| 5 # up arrow
---| 6 # arrow nw-se
---| 7 # arrow ne-sw
---| 8 # arrow e-w
---| 9 # arrow n-s
---| 10 # arrow - all ways
---| 11 # (X) cannot do action
---| 12 # help (? symbol)

---@alias WindowPosition
---| 0 # Stretch to inner size
---| 1 # Scale to inner size, preserving aspect ratio
---| 2 # Stretch to outer size
---| 3 # Scale to outer size, preserving aspect ratio
---| 4 # Top left
---| 5 # Top center
---| 6 # Top right
---| 7 # Center right
---| 8 # Bottom right
---| 9 # Bottom center
---| 10 # Bottom left
---| 11 # Center left
---| 12 # Center
---| 13 # Tile

---Closes an SQLite database.
---@param databaseId string -- Unique database ID assigned during `DatabaseOpen`.
---@return integer -- 0-2
--[[
0: Closed OK<br>
1: Database ID not found<br>
2: Database not open
]]
function DatabaseClose(databaseId) end

---Opens or creates a database. Databases can be held in memory or on disk. You need read or write permissions to access the database, as it is stored on disk as a normal disk file.
---@param databaseId string A unique ID you choose to identify this database for all other operations on it.
---@param filename string The file name on disk. The special filename ":memory:" opens an in-memory database. This can be used to manipulate data (or play with SQL) without actually writing to disk.
---@param flags integer SQLite opening flags:
--[[
Open_ReadOnly = 0x00000001\
Open_ReadWrite = 0x00000002\
Open_Create = 0x00000004\
Open_DeleteOnClose = 0x00000008\
Open_Exclusive = 0x00000010\
Open_Main_db = 0x00000100\
Open_Temp_db = 0x00000200\
Open_Transient_db = 0x00000400\
Open_Main_Journal = 0x00000800\
Open_Temp_Journal = 0x00001000\
Open_SubJournal = 0x00002000\
Open_Master_Journal = 0x00004000\
Open_NoMutex = 0x00008000\
Open_FullMutex = 0x00010000

Warning - the SQLite documentation suggests that you choose one of:

Open_ReadOnly (1)\
Open_ReadWrite (2) or\
Open_ReadWrite + Open_Create (6)

If not, the behaviour of the open may be undefined.

Use [`DatabaseClose`](lua://DatabaseCloses) to close the database when you are finished with it.

If the database is still open when the world file is closed, the database is automatically closed.

It is not an error to re-open the same database ID of an existing, open, database, provided the disk file name is the same. This will be treated as a no-operation, so that triggers and aliases can open the database without having to check first if it was already open.
]]
---@return SQLite3Code -- The return codes are available in the [`sqlite3`](lua://sqlite3) table in Lua.
---@see SQLite3Code
function DatabaseOpen(databaseId, filename, flags) end

---@alias GetInfoBoolean 106
---@alias GetInfoInteger 239 | 240 | 272 | 273 | 274 | 275 | 280 | 281
---@alias GetInfoString 72

---Gets details about the current world.
---@param infoType integer
---@return nil
---@overload fun(infoType: GetInfoBoolean): boolean
---@overload fun(infoType: GetInfoInteger): integer
---@overload fun(infoType: GetInfoString): string
---@overload fun(infoType: 268): OperatingSystem | nil
function GetInfo(infoType) end

---@alias GetLineInfoBoolean 4 | 5 | 6 | 7 | 8
---@alias GetLineInfoInteger 2 | 3 | 10 | 11
---@alias GetLineInfoString 1
---@alias GetLineInfoTimestamp 9 | 13

---Gets details about any line in the output buffer (window).
---@param lineNumber integer
---@param infoType integer
---@return nil
---@overload fun(infoType: GetLineInfoBoolean): boolean
---@overload fun(infoType: GetLineInfoInteger): integer
---@overload fun(infoType: GetLineInfoString): string
---@overload fun(infoType: GetLineInfoTimestamp): integer
function GetLineInfo(lineNumber, infoType) end

---@alias GetPluginInfoBoolean 15 | 16 | 17
---@alias GetPluginInfoInteger 9 | 10 | 11 | 12 | 21 | 25
---@alias GetPluginInfoNumber 18 | 19 | 24
---@alias GetPluginInfoString 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 20 | 23
---@alias GetPluginInfoTimestamp 13 | 14 | 22

---Gets details about a specified plugin.
---@param pluginId string
---@param infoType integer
---@return nil
---@overload fun(pluginId: string, infoType: GetPluginInfoBoolean): boolean
---@overload fun(pluginId: string, infoType: GetPluginInfoInteger): integer
---@overload fun(pluginId: string, infoType: GetPluginInfoNumber): number
---@overload fun(pluginId: string, infoType: GetPluginInfoString): string
---@overload fun(pluginId: string, infoType: GetPluginInfoTimestamp): integer
function GetPluginInfo(pluginId, infoType) end

---@alias GetStyleInfoBoolean 8 | 9 | 10 | 11 | 12 | 13
---@alias GetStyleInfoColour 14 | 15
---@alias GetStyleInfoInteger 2 | 3
---@alias GetStyleInfoString 1 | 5 | 6

---Gets details about a specified style run for a specified line in the output window
---@param lineNumber integer
---@param styleNumber integer
---@param infoType integer
---@return nil
---@overload fun(infoType: GetStyleInfoBoolean): boolean
---@overload fun(infoType: GetStyleInfoInteger): integer
---@overload fun(infoType: GetStyleInfoColour): integer
---@overload fun(infoType: GetStyleInfoString): string
---@overload fun(infoType: 4): LinkAction
function GetStyleInfo(lineNumber, styleNumber, infoType) end

---@alias GetTimerInfoBoolean 6 | 7 | 8 | 14 | 15 | 16 | 17 | 18 | 23 | 24 | 26
---@alias GetTimerInfoInteger 1 | 2 | 3 | 20 | 21
---@alias GetTimerInfoString 4 | 5 | 19 | 22
---@alias GetTimerInfoTimestamp  11 | 12

---@param timerId string
---@param infoType integer
---@return nil
---@overload fun(timerId: string, infoType: GetTimerInfoBoolean): boolean
---@overload fun(timerId: string, infoType: GetTimerInfoInteger): integer
---@overload fun(timerId: string, infoType: GetTimerInfoString): string
---@overload fun(timerId: string, infoType: GetTimerInfoTimestamp): integer | nil
---@overload fun(timerId: string, infoType: 13): integer | nil
function GetTimerInfo(timerId, infoType) end

---Returns a positive integer which is unique for the current session across all worlds. This number resets for each new session, so these numbers are only unique per session, not indefinitely.
---Note that the number will eventually "wrap around" and thus, if you generate a lot of them (ie. 2,147,483,647 of them), they will no longer be unique.
---@return integer
function GetUniqueNumber() end

---Gets the corresponding MUSHclient version of this API.
---@return string
function Version() end

---@alias WindowInfoBoolean 5 | 6
---@alias WindowInfoColour 9
---@alias WindowInfoInteger 1 | 2 | 3 | 4 | 8 | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18
---@alias WindowInfoString 19 | 20
---@alias WindowInfoTimestamp 21

---Gets information about a miniwindow.
---@param windowName string
---@param infoType integer
---@return nil
---@overload fun(windowName: string, infoType: WindowInfoBoolean): boolean
---@overload fun(windowName: string, infoType: WindowInfoColour): integer
---@overload fun(windowName: string, infoType: WindowInfoInteger): integer
---@overload fun(windowName: string, infoType: WindowInfoString): string
---@overload fun(windowName: string, infoType: WindowInfoTimestamp): integer
---@overload fun(windowName: string, infoType: 7): WindowPosition
function WindowInfo(windowName, infoType) end

---Sends text to the world.
---@param ...string
---@return ApiCode
function Send(...) end

---Sends text to the world without echoing it to output.
---@param ...string
---@return ApiCode
function SendNoEcho(...) end

---Sends a packet to the server.
---@param packet string
---@return ApiCode
function SendPkt(packet) end

---Gets the value of a string configuration option.
---@param optionName string
---@return string
function GetAlphaOption(optionName) end

---Gets the list of world string options.
---@return string[]
function GetAlphaOptionList() end

---Gets the current value of a world option, that is, the value it currently has.
---@param optionName string
---@return boolean | number | string | nil
function GetCurrentValue(optionName) end

---Gets value of a world numeric option. Many options are booleans (true or false). The number zero represents false (option not set) and the number one represents true (option set).
---@param optionName string
---@return number
function GetOption(optionName) end

---Gets the list of world numeric options.
---@return string[]
function GetOptionList() end

---Set value of a named world strsing option.
---@param optionName string
---@param value string
---@return ApiCode
function SetAlphaOption(optionName, value) end

---Set value of a named world numeric option.
---@param optionName string
---@param value number | boolean
---@return ApiCode
function SetOption(optionName, value) end

---Converts a colour name to a BBGGRR color code.
---@param colourName string
---@return integer
function ColourNameToRGB(colourName) end

---Sends a message to the output window in specified colours.
---@param foreground ColourParam
---@param background ColourParam
---@param text any Message content.
---@param ...any More triples of the above.
function ColourNote(foreground, background, text, ...) end

---Sends a message to the output window in specified colours without inserting a newline.
---@param foreground ColourParam
---@param background ColourParam
---@param text any Message content.
---@param ...any More triples of the above.
function ColourTell(foreground, background, text, ...) end

---Gets the number of lines in the output window.
---@return integer
function GetLinesInBufferCount() end

---Creates a hyperlink in the output window.
---@param action string Action that will be sent if the link is clicked.
---@param text string Content of hyperlink.
---@param hint string Prompt if the mouse hovers over the hyperlink. If empty, defaults to `action`.
---@param foreground ColourParam
---@param background ColourParam
---@param isUrl? boolean|0|1 Whether to open the link in a browser, rather than treating it as a command. Default: false.
---@param noUnderline? boolean|0|1 Whether to remove the underline of the link. Default: false.
function Hyperlink(action, text, hint, foreground, background, isUrl, noUnderline) end

---Sends a message to the output window.
---@param ...any Message content.
function Note(...) end

---Invokes a colour picker dialog to select a colour.
---@param suggested? integer Starting colour code suggestion.
---@returns integer
function PickColour(suggested) end

---Converts a BBGGRR colour code to its equivalent name.
---@param colourCode integer
---@return string
function RGBColourToName(colourCode) end

---Sets the clipboard contents.
---@param ...string
function SetClipboard(...) end

---Changes the shape of the mouse cursor.
---@param cursor WindowCursorShape
---@return ApiCode
function SetCursor(cursor) end

---Sets text on the status bar at the bottom of the screen.
---@param message string
function SetStatus(message) end

---Sends a message to the output window without appending a newline.
---@param ...any Message content.
function Tell(...) end

---Broadcasts a message to all installed plugins.
---@param code integer Arbitrary message code.
---@param text string Message text.
---@return ApiCode
function BroadcastPlugin(code, text) end

---@param pluginId string
---@param routine string
---@param ...any
---@return ...
function CallPlugin(pluginId, routine, ...) end

---Enables or disables a plugin.
---@param pluginId string
---@param enable? boolean|0|1 Default: true.
---@return ApiCode
function EnablePlugin(pluginId, enable) end

---Gets the ID of the active plugin.
---@return string
function GetPluginID() end

---Checks if a plugin exposes a callback.
---@param pluginId string
---@param routine string
---@return ApiCode
function PluginSupports(pluginId, routine) end

---Creates an alias.
---@param name string
---@param pattern string
---@param text string
---@param flags integer
---@param routine? string
---@return ApiCode
function AddAlias(name, pattern, text, flags, routine) end

---Creates a timer.
---@param name string
---@param hour integer
---@param minute integer
---@param second number
---@param text string
---@param flags integer
---@param routine? string
---@return ApiCode
function AddTimer(name, hour, minute, second, text, flags, routine) end

---Creates a trigger.
---@param name string
---@param pattern string
---@param text string
---@param flags integer
---@param colour integer
---@param wildcard integer
---@param soundFile string
---@param routine? string
---@param target? SendTo
---@param sequence? integer
---@return ApiCode
function AddTrigger(name, pattern, text, flags, colour, wildcard, soundFile, routine, target, sequence) end

AddTriggerEx = AddTrigger

---Deletes an alias by name.
---@param name string
---@return ApiCode
function DeleteAlias(name) end

---Deletes aliases by group.
---@param group string
---@return integer -- Number of deleted aliases.
function DeleteAliases(group) end

---Deletes a timer by name.
---@param name string
---@return ApiCode
function DeleteTimer(name) end

---Deletes timers by group.
---@param group string
---@return integer -- Number of deleted timers.
function DeleteTimers(group) end

---Deletes a trigger by name.
---@param name string
---@return ApiCode
function DeleteTrigger(name) end

---Deletes triggers by group.
---@param group string
---@return integer -- Number of deleted triggers.
function DeleteTriggers(group) end

---Deletes a persistent variable.
---@param key string
---@return ApiCode
function DeleteVariable(key) end

---Sends a command after a delay.
---@param seconds number Delay in seconds.
---@param text string Command content.
---@return ApiCode
function DoAfter(seconds, text) end

---Creates a note after a delay.
---@param seconds number Delay in seconds.
---@param text string Note content.
---@return ApiCode
function DoAfterNote(seconds, text) end

---Adds a one-shot, temporary, timer to carry out some special action.
---@param seconds number Delay in seconds.
---@param text string Action content.
---@param target SendTo Action send target.
---@return ApiCode
function DoAfterSpecial(seconds, text, target) end

---Sends a command to speedwalk a delay.
---@param seconds number Delay in seconds.
---@param text string Command content.
---@return ApiCode
function DoAfterSpeedwalk(seconds, text) end

---Enables or disables an alias by name.
---@param name string Alias name.
---@param enable? boolean|0|1 Default: true.
---@return ApiCode
function EnableAlias(name, enable) end

---Enables or disables aliases by group.
---@param group string Group name.
---@param enable? boolean|0|1 Default: true.
---@return ApiCode
function EnableAliasGroup(group, enable) end

---Enables or disables a timer by name.
---@param name string Timer name.
---@param enable? boolean|0|1 Default: true.
---@return ApiCode
function EnableTimer(name, enable) end

---Enables or disables timers by group.
---@param group string Group name.
---@param enable? boolean|0|1 Default: true.
---@return ApiCode
function EnableTimerGroup(group, enable) end


---Enables or disables a trigger by name.
---@param name string Trigger name.
---@param enable? boolean|0|1 Default: true.
---@return ApiCode
function EnableTrigger(name, enable) end

---Enables or disables triggers by group.
---@param group string Group name.
---@param enable? boolean|0|1 Default: true.
---@return ApiCode
function EnableTriggerGroup(group, enable) end

---Checks whether an alias exists with the specified name.
---@param name string Alias name.
---@return ApiCode
function IsAlias(name) end

---Checks whether a timer exists with the specified name.
---@param name string Timer name.
---@return ApiCode
function IsTimer(name) end

---Checks whether a trigger exists with the specified name.
---@param name string Trigger name.
---@return ApiCode
function IsTrigger(name) end

---Creates a regular expression from a wildcard pattern.
---@param pattern string Wildcard pattern.
---@return string
function MakeRegularExpression(pattern) end

---Sets the value of a named trigger option.
---@param name string Trigger name.
---@param optionName string Option name.
---@param option boolean | integer | string Option value.
---@return ApiCode
function SetTriggerOption(name, optionName, option) end

---Stops trigger evaluation.
---@param _allPlugins? boolean|0|1 Unused.
function StopEvaluatingTriggers(_allPlugins) end

---@param channel integer
---@param fileName string
---@param loop? boolean|0|1 Default: false.
---@param volume? number Default: 0.
---@param pan? 0 Unused.
---@return ApiCode
function PlaySound(channel, fileName, loop, volume, pan) end

---@param channel integer
---@param buffer string Sound bytes buffer.
---@param loop? boolean|0|1 Default: false.
---@param volume? number Default: 0.
---@param pan? 0 Unused.
---@return ApiCode
function PlaySoundMemory(channel, buffer, loop, volume, pan) end

---@param channel? integer
---@return ApiCode
function StopSound(channel) end

---Gets the contents of a persisted variable.
---@param variableName string
---@return string | nil
function GetVariable(variableName) end

---Gets the contents of a persisted variable for a different plugin.
---@param pluginId string
---@param variableName string
---@return string | nil
function GetPluginVariable(pluginId, variableName) end

---Sets the contents of a persisted variable.
---@param variableName string
---@param value string
---@return ApiCode
function SetVariable(variableName, value) end

---Does nothing. Provided for backward compatibility.
function Redraw() end

---Does nothing. Provided for backward compatibility.
function Repaint() end

---Specifies the size of the rectangle in which text is displayed in the output window.
---@param left integer
---@param top integer
---@param right integer
---@param bottom integer
---@param offset integer
---@param borderColour ColourParam
---@param borderWidth integer
---@param outsideFillColour ColourParam
---@param outsideFillStyle WindowBrushStyle
---@return ApiCode
function TextRectangle(left, top, right, bottom, offset, borderColour, borderWidth, outsideFillColour, outsideFillStyle) end

---Draws ellipses, filled rectangles, round rectangles, chords, pies in a miniwindow.
---@param windowName string
---@param action
---| 1 # Ellipse
---| 2 # Rectangle
---| 3 # Rounded rectangle
---@param left number
---@param top number
---@param right number
---@param bottom number
---@param penColour ColourParam
---@param penStyle integer
---@param penWidth number
---@param brushColour ColourParam
---@param brushStyle WindowBrushStyle
---@return ApiCode
function WindowCircleOp(windowName, action, left, top, right, bottom, penColour, penStyle, penWidth, brushColour, brushStyle) end

--- Creates or recreates a miniwindow.
---@param name string
---@param left integer
---@param top integer
---@param width integer
---@param height integer
---@param position WindowPosition
---@param flags integer
---@param background ColourParam
---@return ApiCode
function WindowCreate(name, left, top, width, height, position, flags, background) end

--- Draws an image in a miniwindow.
---@param windowName string
---@param imageId string
---@param left number
---@param top number
---@param right number
---@param bottom number
---@param mode
---| 1 # Copy without stretching to the destination position. The image is not clipped, so only the Left and Top parameters are used - the full image is copied to that position.
---| 2 # Stretch or shrink the image appropriately to fit into the rectangle: Left, Top, Right, Bottom.
---| 3 # Copy without stretching to the position Left, Top. However this is a transparent copy, where the pixel at the left,top corner (pixel position 0,0) is considered the transparent colour. Any pixels that exactly match that colour are not copied.
---@param srcLeft? number
---@param srcTop? number
---@param srcRight? number
---@param srcBottom? number
---@return ApiCode
function WindowDrawImage(windowName, imageId, left, top, right, bottom, mode, srcLeft, srcTop, srcRight, srcBottom) end

--- Draws an image in a miniwindow with partial opacity.
---@param windowName string
---@param imageId string
---@param left number
---@param top number
---@param right number
---@param bottom number
---@param opacity number
---@param srcLeft? number
---@param srcTop? number
---@return ApiCode
function WindowDrawImageAlpha(windowName, imageId, left, top, right, bottom, opacity, srcLeft, srcTop) end

---Performs a filtering operation over part of the miniwindow.
---@param windowName string
---@param left number
---@param top number
---@param right number
---@param bottom number
---@param filter integer
---@param option? number
---@return ApiCode
function WindowFilter(windowName, left, top, right, bottom, filter, option) end

--long WindowFont(BSTR WindowName, BSTR FontId, BSTR FontName, double Size, BOOL Bold, BOOL Italic, BOOL Underline, BOOL Strikeout, short Charset, short PitchAndFamily);

---Loads a font into a miniwindow.
---@param windowName string
---@param fontId string
---@param fontName string
---@param pointSize number
---@param bold? boolean|0|1 Default: false.
---@param italic? boolean|0|1 Default: false.
---@param underline? boolean|0|1 Default: false.
---@param strikeout? boolean|0|1 Default: false.
---@param _charset? integer Unused.
---@param pitchAndFamily? integer Default: 0.
---@return ApiCode
function WindowFont(windowName, fontId, fontName, pointSize, bold, italic, underline, strikeout, _charset, pitchAndFamily) end

---@alias WindowFontInfoBoolean 16 | 17 | 18
---@alias WindowFontInfoInteger 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 19
---@alias WindowFontInfoString 21

---Gets information about a font in a miniwindow.
---@param windowName string
---@param fontId string
---@param infoType integer
---@return nil
---@overload fun(windowName: string, fontId: string, infoType: WindowFontInfoBoolean): boolean
---@overload fun(windowName: string, fontId: string, infoType: WindowFontInfoInteger): integer
---@overload fun(windowName: string, fontId: string, infoType: WindowFontInfoString): string
function WindowFontInfo(windowName, fontId, infoType) end

---Paints a gradient in a miniwindow.
---@param windowName string
---@param left number
---@param top number
---@param right number
---@param bottom number
---@param startColour ColourParam
---@param endColour ColourParam
---@param mode
---| 1 # Horizontal
---| 2 # Vertical
---@return ApiCode
function WindowGradient(windowName, left, top, right, bottom, startColour, endColour, mode) end

---Creates an image from another miniwindow.
---@param windowName string
---@param imageId string
---@param sourceWindowName string
---@return ApiCode
function WindowImageFromWindow(windowName, imageId, sourceWindowName) end

---Draws a line in a miniwindow.
---@param windowName string
---@param x1 number
---@param y1 number
---@param x2 number
---@param y2 number
---@param penColour ColourParam
---@param penStyle integer
---@param penWidth number
---@return ApiCode
function WindowLine(windowName, x1, y1, x2, y2, penColour, penStyle, penWidth) end

---Loads an image from a file into a miniwindow.
---@param windowName string
---@param imageId string
---@param fileName string
---@return ApiCode
function WindowLoadImage(windowName, imageId, fileName) end

---Creates a pop-up menu inside a miniwindow.
---@param windowName string
---@param left integer
---@param top integer
---@param items string
---@return number | string
function WindowMenu(windowName, left, top, items) end

---Draws/fills a polygon in a miniwindow.
---@param windowName string
---@param points string
---@param penColour ColourParam
---@param penStyle integer
---@param penWidth number
---@param brushColour ColourParam
---@param brushStyle WindowBrushStyle
---@param close? boolean|0|1 Default: false.
---@param winding? boolean|0|1 Default: false.
---@return ApiCode
function WindowPolygon(windowName, points, penColour, penStyle, penWidth, brushColour, brushStyle, close, winding) end

---Moves a miniwindow.
---@param windowName string
---@param left integer
---@param top integer
---@param position WindowPosition
---@param flags integer
---@return ApiCode
function WindowPosition(windowName, left, top, position, flags) end

---Draws a rectangle in a miniwindow.
---@param windowName string
---@param action
---| 1 # Frame by a single pixel wide line
---| 2 # Fill the entire rectangle
---| 4 # Draw a "3D-style" rectangle in two colors, a single pixel wide
---@param left number
---@param top number
---@param right number
---@param bottom number
---@param colour ColourParam
---@return ApiCode
---@overload fun(windowName: string, action: 3, left: number, top: number, right: number, bottom: number): ApiCode
---@overload fun(windowName: string, action: 4, left: number, top: number, right: number, bottom: number, colour1: integer, colour2: integer): ApiCode
---@overload fun(windowName: string, action: 5, left: number, top: number, right: number, bottom: number, frame: WindowButtonFrame, flags: integer): ApiCode;
function WindowRectOp(windowName, action, left, top, right, bottom, colour) end

--BSTR WindowName, long Width, long Height, long BackgroundColour

--- Resizes a miniwindow.
---@param windowName string
---@param width integer
---@param height integer
---@param background ColourParam
---@return ApiCode
function WindowResize(windowName, width, height, background) end

---Sets the z-order of a miniwindow.
---@param windowName string
---@param zOrder integer
---@return ApiCode
function WindowSetZOrder(windowName, zOrder) end

---Shows or hides a miniwindow.
---@param windowName string
---@param show? boolean|0|1 Default: true.
---@return ApiCode
function WindowShow(windowName, show) end

--long WindowText(BSTR WindowName, BSTR FontId, BSTR Text, long Left, long Top, long Right, long Bottom, long Colour, BOOL Unicode);

---Draws text into a miniwindow.
---@param windowName string
---@param fontId string
---@param text string
---@param left number
---@param top number
---@param right number
---@param bottom number
---@param colour ColourParam
---@param unicode? boolean|0|1 Default: false.
---@return ApiCode
function WindowText(windowName, fontId, text, left, top, right, bottom, colour, unicode) end

---Calculates the width of text in a miniwindow.
---@param windowName string
---@param fontId string
---@param text string
---@param unicode? boolean|0|1 Default: false.
---@return ApiCode
function WindowTextWidth(windowName, fontId, text, unicode) end

---Adds a mouse-interactive hotspot to a miniwindow.
---@param windowName string
---@param hotspotId string
---@param left integer
---@param top integer
---@param right integer
---@param bottom integer
---@param mouseOver? string
---@param cancelMouseOver? string
---@param mouseDown? string
---@param cancelMouseDown? string
---@param mouseUp? string
---@param tooltipText? string
---@param cursor? WindowCursorShape Default: 0 (arrow cursor)
---@param flags? integer
---@return ApiCode
function WindowAddHotspot(windowName, hotspotId, left, top, right, bottom, mouseOver, cancelMouseOver, mouseDown, cancelMouseDown, mouseUp, tooltipText, cursor, flags) end

---@param windowName string
---@param hotspotId string
---@return ApiCode
function WindowDeleteHotspot(windowName, hotspotId) end

---Sets drag event handlers for a hotspot.
---@param windowName string
---@param hotspotId string
---@param moveCallback? string
---@param releaseCallback? string
---@param flags? 0 Unused.
---@return ApiCode
function WindowDragHandler(windowName, hotspotId, moveCallback, releaseCallback, flags) end

---Moves a hotspot.
---@param windowName string
---@param hotspotId string
---@param left integer
---@param top integer
---@param right integer
---@param bottom integer
---@return ApiCode
function WindowMoveHotspot(windowName, hotspotId, left, top, right, bottom) end

---Sets scroll wheel handler for a hotspot.
---@param windowName string
---@param hotspotId string
---@param moveCallback? string
---@return ApiCode
function WindowScrollwheelHandler(windowName, hotspotId, moveCallback) end

---Sets a background image for the output window.
---@param fileName string
---@param mode WindowPosition
---@return ApiCode
function SetBackgroundImage(fileName, mode) end

---Sets the background colour of the client frame.
---@param colour ColourParam
function SetFrameBackgroundColour(colour) end
