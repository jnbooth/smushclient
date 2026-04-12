---@meta

---@enum custom_colour
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

---@enum error_code
error_code = {
    eOK = 0,                             ---No error
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

---@type { [error_code]: string }
error_desc = error_desc

---@enum miniwin.blend
miniwin = {
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
}


---@enum miniwin.brush
miniwin = {
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
}

---@enum miniwin.cursor
miniwin = {
    cursor_none = -1,       --- no cursor
    cursor_arrow = 0,       --- arrow
    cursor_hand = 1,        --- hand
    cursor_ibeam = 2,       --- I-beam
    cursor_plus = 3,        --- + symbol
    cursor_wait = 4,        --- wait (hour-glass)
    cursor_up = 5,          --- up arrow
    cursor_nw_se_arrow = 6, --- arrow nw-se
    cursor_ne_sw_arrow = 7, --- arrow ne-sw
    cursor_ew_arrow = 8,    --- arrow e-w
    cursor_ns_arrow = 9,    --- arrow n-s
    cursor_both_arrow = 10, --- arrow - all ways
    cursor_x = 11,          --- (X) cannot do action
    cursor_help = 12,       --- help (? symbol)
}

---@enum miniwin.filter
miniwin = {
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
}

---@enum miniwin.pos
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
}

---@enum miniwin
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

---@enum operating_system
operating_system = {
    Windows = 2,
    MacOS = 100,
    Linux = 200,
}

---@enum sendto
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

---@enum alias_flag
alias_flag = {
    Enabled = 0x1,            ---Enable
    KeepEvaluating = 0x8,     ---Keep evaluating
    IgnoreAliasCase = 0x20,   ---Ignore case when matching
    OmitFromLogFile = 0x40,   ---Omit from log file
    RegularExpression = 0x80, ---Uses regular expression
    ExpandVariables = 0x200,  ---Expand variables like @direction
    Replace = 0x400,          ---Replace existing of same name
    AliasSpeedWalk = 0x800,   ---Interpret send string as a speed walk string
    AliasQueue = 0x1000,      ---Queue this alias for sending at the speedwalking delay interval
    AliasMenu = 0x2000,       ---This alias appears on the alias menu
    Temporary = 0x4000,       ---Temporary - do not save to world file
    OneShot = 0x8000,         ---If set, only fires once
}

---@enum timer_flag
timer_flag = {
    Enabled = 0x1,           ---Enable
    AtTime = 0x2,            ---If not set, time is "every"
    OneShot = 0x4,           ---If set, only fires once
    TimerSpeedWalk = 0x8,    ---Timer does a speed walk when it fires
    TimerNote = 0x10,        ---Timer does a world.Note when it fires
    ActiveWhenClosed = 0x20, ---Timer fires even when world is disconnected
    Replace = 0x400,         ---Replace existing of same name
    Temporary = 0x4000,      ---Temporary - do not save to world file
}

---@enum trigger_flag
trigger_flag = {
    Enabled = 0x1,             ---Enable
    OmitFromLog = 0x2,         ---Omit from log file
    OmitFromOutput = 0x4,      ---Omit from output
    KeepEvaluating = 0x8,      ---Keep evaluating
    IgnoreCase = 0x10,         ---Ignore case when matching
    RegularExpression = 0x20,  ---Uses regular expression
    ExpandVariables = 0x200,   ---Expand variables like @direction
    Replace = 0x400,           ---Replace existing of same name
    LowercaseWildcard = 0x800, ---Wildcards forced to lower-case
    Temporary = 0x4000,        ---Temporary - do not save to world file
    OneShot = 0x8000,          ---If set, only fires once
}
