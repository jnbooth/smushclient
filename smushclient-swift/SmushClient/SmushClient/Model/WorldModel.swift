import SwiftUI

private let defaultHyperlinkColor = NSColor.linkColor.usingColorSpace(.sRGB)!
private let defaultChatColor = NSColor(red: 1, green: 1, blue: 1, alpha: 1)
private let defaultCustomColor = NSColor(red: 0, green: 0, blue: 0, alpha: 1)
private let defaultInputColor = NSColor(red: 0.5, green: 0.5, blue: 0.5, alpha: 1)
private let defaultEchoColor = defaultInputColor
private let defaultNoteTextColor = NSColor(red: 0, green: 0.5, blue: 1, alpha: 1)
private let defaultCustomNames = [
  "Custom1", "Custom2", "Custom3", "Custom4", "Custom5", "Custom6", "Custom7", "Custom8",
  "Custom9", "Custom10", "Custom11", "Custom12", "Custom13", "Custom14", "Custom15", "Custom16",
]

@Observable
class WorldModel {
  static let defaultFont = NSFont.monospacedSystemFont(
    ofSize: NSFont.systemFontSize, weight: .medium)

  var name: String = ""
  var site: String = ""
  var port: UInt16 = 0
  var proxy_type: ProxyType = .None
  var proxy_server: String = ""
  var proxy_port: UInt16 = 0
  var proxy_username: String = ""
  var proxy_password: String = ""
  var proxy_password_base64: Bool = true
  var save_world_automatically: Bool = false
  var player: String = ""
  var password: String = ""
  var connect_method: AutoConnect = .None
  var connect_text: String = ""
  var log_file_preamble: String = ""
  var log_file_postamble: String = ""
  var log_format: LogFormat = .Text
  var log_output: Bool = true
  var log_input: Bool = true
  var log_notes: Bool = true
  var log_mode: LogMode = .Append
  var auto_log_file_name: String = ""
  var log_preamble_output: String = ""
  var log_preamble_input: String = ""
  var log_preamble_notes: String = ""
  var log_postamble_output: String = ""
  var log_postamble_input: String = ""
  var log_postamble_notes: String = ""
  var timers: [TimerModel] = []
  var enable_timers: Bool = true
  var treeview_timers: Bool = true
  var chat_name: String = ""
  var auto_allow_snooping: Bool = false
  var accept_chat_connections: Bool = false
  var chat_port: UInt16 = 0
  var validate_incoming_chat_calls: Bool = false
  var chat_colors: ColorPairModel = ColorPairModel(foreground: defaultChatColor)
  var ignore_chat_colors: Bool = false
  var chat_message_prefix: String = ""
  var chat_max_lines_per_message: UInt = 0
  var chat_max_bytes_per_message: UInt = 0
  var auto_allow_files: Bool = false
  var chat_file_save_directory: String = ""
  var notes: String = ""
  var beep_sound: String = ""
  var pixel_offset: Int16 = 5
  var line_spacing: Float = 1.0
  var output_font: NSFont = defaultFont
  var use_default_output_font: Bool = true
  var show_bold: Bool = true
  var show_italic: Bool = true
  var show_underline: Bool = true
  var new_activity_sound: String = ""
  var max_output_lines: UInt = 5000
  var wrap_column: UInt16 = 80
  var line_information: Bool = true
  var start_paused: Bool = false
  var auto_pause: Bool = true
  var unpause_on_send: Bool = true
  var flash_taskbar_icon: Bool = false
  var disable_compression: Bool = false
  var indent_paras: Bool = true
  var naws: Bool = false
  var carriage_return_clears_line: Bool = false
  var utf_8: Bool = true
  var auto_wrap_window_width: Bool = false
  var show_connect_disconnect: Bool = true
  var copy_selection_to_clipboard: Bool = false
  var auto_copy_to_clipboard_in_html: Bool = false
  var convert_ga_to_newline: Bool = false
  var terminal_identification: String = "mushclient"
  var use_mxp: UseMxp = .Command
  var detect_pueblo: Bool = true
  var hyperlink_color: NSColor = defaultHyperlinkColor
  var use_custom_link_color: Bool = false
  var mud_can_change_link_color: Bool = true
  var underline_hyperlinks: Bool = true
  var mud_can_remove_underline: Bool = false
  var hyperlink_adds_to_command_history: Bool = true
  var echo_hyperlink_in_output_window: Bool = true
  var ignore_mxp_color_changes: Bool = false
  var send_mxp_afk_response: Bool = true
  var mud_can_change_options: Bool = true
  var use_default_colors: Bool = true
  var ansi_colors: AnsiColors = defaultAnsiColors
  var custom_names: [String] = defaultCustomNames
  var custom_colors: [ColorPairModel] = Array(
    repeating: ColorPairModel(foreground: defaultCustomColor), count: 16)
  var triggers: [TriggerModel] = []
  var enable_triggers: Bool = true
  var enable_trigger_sounds: Bool = true
  var treeview_triggers: Bool = true
  var display_my_input: Bool = true
  var echo_colors: ColorPairModel = ColorPairModel(foreground: defaultEchoColor)
  var enable_speed_walk: Bool = false
  var speed_walk_prefix: String = "#"
  var speed_walk_filler: String = "a"
  var speed_walk_delay: UInt32 = 20
  var enable_command_stack: Bool = false
  var command_stack_character: String = "#"
  var input_colors: ColorPairModel = ColorPairModel(foreground: defaultInputColor)
  var input_font: NSFont = defaultFont
  var use_default_input_font: Bool = true
  var enable_spam_prevention: Bool = false
  var spam_line_count: UInt = 20
  var spam_message: String = "look"
  var auto_repeat: Bool = false
  var lower_case_tab_completion: Bool = false
  var translate_german: Bool = false
  var translate_backslash_sequences: Bool = false
  var keep_commands_on_same_line: Bool = false
  var no_echo_off: Bool = false
  var tab_completion_lines: UInt = 200
  var tab_completion_space: Bool = false
  var double_click_inserts: Bool = false
  var double_click_sends: Bool = false
  var escape_deletes_input: Bool = false
  var save_deleted_command: Bool = false
  var confirm_before_replacing_typing: Bool = true
  var arrow_keys_wrap: Bool = false
  var arrows_change_history: Bool = true
  var arrow_recalls_partial: Bool = false
  var alt_arrow_recalls_partial: Bool = false
  var ctrl_z_goes_to_end_of_buffer: Bool = false
  var ctrl_p_goes_to_previous_command: Bool = false
  var ctrl_n_goes_to_next_command: Bool = false
  var history_lines: UInt = 1000
  var aliases: [AliasModel] = []
  var enable_aliases: Bool = true
  var treeview_aliases: Bool = true
  var keypad_enable: Bool = true
  var keypad_shortcuts: [String: String] = [:]
  var enable_auto_say: Bool = false
  var autosay_exclude_non_alpha: Bool = false
  var autosay_exclude_macros: Bool = false
  var auto_say_override_prefix: String = "-"
  var auto_say_string: String = ""
  var re_evaluate_auto_say: Bool = false
  var paste_line_preamble: String = ""
  var paste_line_postamble: String = ""
  var paste_delay: UInt32 = 0
  var paste_delay_per_lines: UInt32 = 1
  var paste_commented_softcode: Bool = false
  var paste_echo: Bool = false
  var confirm_on_paste: Bool = true
  var send_line_preamble: String = ""
  var send_line_postamble: String = ""
  var send_delay: UInt32 = 0
  var send_delay_per_lines: UInt32 = 1
  var send_commented_softcode: Bool = false
  var send_echo: Bool = false
  var confirm_on_send: Bool = true
  var world_script: String = ""
  var script_prefix: String = ""
  var enable_scripts: Bool = true
  var warn_if_scripting_inactive: Bool = true
  var edit_script_with_notepad: Bool = true
  var script_editor: String = "System"
  var script_reload_option: ScriptRecompile = .Confirm
  var script_errors_to_output_window: Bool = false
  var note_text_color: NSColor = defaultNoteTextColor
  var plugins: [String] = []

  init() {}

  init(_ world: World) {
    name = world.name.toString()
    site = world.site.toString()
    port = world.port
    proxy_type = world.proxy_type
    proxy_server = world.proxy_server.toString()
    proxy_port = world.proxy_port
    proxy_username = world.proxy_username.toString()
    proxy_password = world.proxy_password.toString()
    proxy_password_base64 = world.proxy_password_base64
    save_world_automatically = world.save_world_automatically
    player = world.player.toString()
    password = world.password.toString()
    connect_method = world.connect_method
    connect_text = world.connect_text.toString()
    log_file_preamble = world.log_file_preamble.toString()
    log_file_postamble = world.log_file_postamble.toString()
    log_format = world.log_format
    log_output = world.log_output
    log_input = world.log_input
    log_notes = world.log_notes
    log_mode = world.log_mode
    auto_log_file_name = world.auto_log_file_name.toString()
    log_preamble_output = world.log_preamble_output.toString()
    log_preamble_input = world.log_preamble_input.toString()
    log_preamble_notes = world.log_preamble_notes.toString()
    log_postamble_output = world.log_postamble_output.toString()
    log_postamble_input = world.log_postamble_input.toString()
    log_postamble_notes = world.log_postamble_notes.toString()
    timers = fromVec(world.timers, by: TimerModel.init)
    enable_timers = world.enable_timers
    treeview_timers = world.treeview_timers
    chat_name = world.chat_name.toString()
    auto_allow_snooping = world.auto_allow_snooping
    accept_chat_connections = world.accept_chat_connections
    chat_port = world.chat_port
    validate_incoming_chat_calls = world.validate_incoming_chat_calls
    chat_colors = ColorPairModel(world.chat_colors)
    ignore_chat_colors = world.ignore_chat_colors
    chat_message_prefix = world.chat_message_prefix.toString()
    chat_max_lines_per_message = world.chat_max_lines_per_message
    chat_max_bytes_per_message = world.chat_max_bytes_per_message
    auto_allow_files = world.auto_allow_files
    chat_file_save_directory = world.chat_file_save_directory.toString()
    notes = world.notes.toString()
    beep_sound = world.beep_sound.toString()
    pixel_offset = world.pixel_offset
    line_spacing = world.line_spacing
    if let output_font = NSFont(world.output_font) {
      self.output_font = output_font
    }
    use_default_output_font = world.use_default_output_font
    show_bold = world.show_bold
    show_italic = world.show_italic
    show_underline = world.show_underline
    new_activity_sound = world.new_activity_sound.toString()
    max_output_lines = world.max_output_lines
    wrap_column = world.wrap_column
    line_information = world.line_information
    start_paused = world.start_paused
    auto_pause = world.auto_pause
    unpause_on_send = world.unpause_on_send
    flash_taskbar_icon = world.flash_taskbar_icon
    disable_compression = world.disable_compression
    indent_paras = world.indent_paras
    naws = world.naws
    carriage_return_clears_line = world.carriage_return_clears_line
    utf_8 = world.utf_8
    auto_wrap_window_width = world.auto_wrap_window_width
    show_connect_disconnect = world.show_connect_disconnect
    copy_selection_to_clipboard = world.copy_selection_to_clipboard
    auto_copy_to_clipboard_in_html = world.auto_copy_to_clipboard_in_html
    convert_ga_to_newline = world.convert_ga_to_newline
    terminal_identification = world.terminal_identification.toString()
    use_mxp = world.use_mxp
    detect_pueblo = world.detect_pueblo
    hyperlink_color = NSColor(world.hyperlink_color)
    use_custom_link_color = world.use_custom_link_color
    mud_can_change_link_color = world.mud_can_change_link_color
    underline_hyperlinks = world.underline_hyperlinks
    mud_can_remove_underline = world.mud_can_remove_underline
    hyperlink_adds_to_command_history = world.hyperlink_adds_to_command_history
    echo_hyperlink_in_output_window = world.echo_hyperlink_in_output_window
    ignore_mxp_color_changes = world.ignore_mxp_color_changes
    send_mxp_afk_response = world.send_mxp_afk_response
    mud_can_change_options = world.mud_can_change_options
    use_default_colors = world.use_default_colors
    ansi_colors = fromVec(world.ansi_colors, by: NSColor.init)
    custom_names = fromVec(world.custom_names, by: { $0.as_str().toString() })
    custom_colors = fromVec(world.custom_colors, by: ColorPairModel.init)
    triggers = fromVec(world.triggers, by: TriggerModel.init)
    enable_triggers = world.enable_triggers
    enable_trigger_sounds = world.enable_trigger_sounds
    treeview_triggers = world.treeview_triggers
    display_my_input = world.display_my_input
    echo_colors = ColorPairModel(world.echo_colors)
    enable_speed_walk = world.enable_speed_walk
    speed_walk_prefix = world.speed_walk_prefix.toString()
    speed_walk_filler = world.speed_walk_filler.toString()
    speed_walk_delay = world.speed_walk_delay
    enable_command_stack = world.enable_command_stack
    command_stack_character = world.command_stack_character.toString()
    input_colors = ColorPairModel(world.input_colors)
    if let input_font = NSFont(world.input_font) {
      self.input_font = input_font
    }
    use_default_input_font = world.use_default_input_font
    enable_spam_prevention = world.enable_spam_prevention
    spam_line_count = world.spam_line_count
    spam_message = world.spam_message.toString()
    auto_repeat = world.auto_repeat
    lower_case_tab_completion = world.lower_case_tab_completion
    translate_german = world.translate_german
    translate_backslash_sequences = world.translate_backslash_sequences
    keep_commands_on_same_line = world.keep_commands_on_same_line
    no_echo_off = world.no_echo_off
    tab_completion_lines = world.tab_completion_lines
    tab_completion_space = world.tab_completion_space
    double_click_inserts = world.double_click_inserts
    double_click_sends = world.double_click_sends
    escape_deletes_input = world.escape_deletes_input
    save_deleted_command = world.save_deleted_command
    confirm_before_replacing_typing = world.confirm_before_replacing_typing
    arrow_keys_wrap = world.arrow_keys_wrap
    arrows_change_history = world.arrows_change_history
    arrow_recalls_partial = world.arrow_recalls_partial
    alt_arrow_recalls_partial = world.alt_arrow_recalls_partial
    ctrl_z_goes_to_end_of_buffer = world.ctrl_z_goes_to_end_of_buffer
    ctrl_p_goes_to_previous_command = world.ctrl_p_goes_to_previous_command
    ctrl_n_goes_to_next_command = world.ctrl_n_goes_to_next_command
    history_lines = world.history_lines
    aliases = fromVec(world.aliases, by: AliasModel.init)
    enable_aliases = world.enable_aliases
    treeview_aliases = world.treeview_aliases
    keypad_enable = world.keypad_enable
    keypad_shortcuts = mappingFromRust(world.keypad_shortcuts)
    enable_auto_say = world.enable_auto_say
    autosay_exclude_non_alpha = world.autosay_exclude_non_alpha
    autosay_exclude_macros = world.autosay_exclude_macros
    auto_say_override_prefix = world.auto_say_override_prefix.toString()
    re_evaluate_auto_say = world.re_evaluate_auto_say
    auto_say_string = world.auto_say_string.toString()
    re_evaluate_auto_say = world.re_evaluate_auto_say
    paste_line_preamble = world.paste_line_preamble.toString()
    paste_line_postamble = world.paste_line_postamble.toString()
    paste_delay = world.paste_delay
    paste_delay_per_lines = world.paste_delay_per_lines
    paste_commented_softcode = world.paste_commented_softcode
    paste_echo = world.paste_echo
    confirm_on_paste = world.confirm_on_paste
    send_line_preamble = world.send_line_preamble.toString()
    send_line_postamble = world.send_line_postamble.toString()
    send_delay = world.send_delay
    send_delay_per_lines = world.send_delay_per_lines
    send_commented_softcode = world.send_commented_softcode
    send_echo = world.send_echo
    confirm_on_send = world.confirm_on_send
    world_script = world.world_script.toString()
    script_prefix = world.script_prefix.toString()
    enable_scripts = world.enable_scripts
    warn_if_scripting_inactive = world.warn_if_scripting_inactive
    edit_script_with_notepad = world.edit_script_with_notepad
    script_editor = world.script_editor.toString()
    script_reload_option = world.script_reload_option
    script_errors_to_output_window = world.script_errors_to_output_window
    note_text_color = NSColor(world.note_text_color)
    plugins = fromVec(world.plugins, by: { $0.as_str().toString() })
  }
}

extension World {
  init(_ world: WorldModel) {
    name = world.name.intoRustString()
    site = world.site.intoRustString()
    port = world.port
    proxy_type = world.proxy_type
    proxy_server = world.proxy_server.intoRustString()
    proxy_port = world.proxy_port
    proxy_username = world.proxy_username.intoRustString()
    proxy_password = world.proxy_password.intoRustString()
    proxy_password_base64 = world.proxy_password_base64
    save_world_automatically = world.save_world_automatically
    player = world.player.intoRustString()
    password = world.password.intoRustString()
    connect_method = world.connect_method
    connect_text = world.connect_text.intoRustString()
    log_file_preamble = world.log_file_preamble.intoRustString()
    log_file_postamble = world.log_file_postamble.intoRustString()
    log_format = world.log_format
    log_output = world.log_output
    log_input = world.log_input
    log_notes = world.log_notes
    log_mode = world.log_mode
    auto_log_file_name = world.auto_log_file_name.intoRustString()
    log_preamble_output = world.log_preamble_output.intoRustString()
    log_preamble_input = world.log_preamble_input.intoRustString()
    log_preamble_notes = world.log_preamble_notes.intoRustString()
    log_postamble_output = world.log_postamble_output.intoRustString()
    log_postamble_input = world.log_postamble_input.intoRustString()
    log_postamble_notes = world.log_postamble_notes.intoRustString()
    timers = intoVec(world.timers, by: Timer.init)
    enable_timers = world.enable_timers
    treeview_timers = world.treeview_timers
    chat_name = world.chat_name.intoRustString()
    auto_allow_snooping = world.auto_allow_snooping
    accept_chat_connections = world.accept_chat_connections
    chat_port = world.chat_port
    validate_incoming_chat_calls = world.validate_incoming_chat_calls
    chat_colors = ColorPair(world.chat_colors)
    ignore_chat_colors = world.ignore_chat_colors
    chat_message_prefix = world.chat_message_prefix.intoRustString()
    chat_max_lines_per_message = world.chat_max_lines_per_message
    chat_max_bytes_per_message = world.chat_max_bytes_per_message
    auto_allow_files = world.auto_allow_files
    chat_file_save_directory = world.chat_file_save_directory.intoRustString()
    notes = world.notes.intoRustString()
    beep_sound = world.beep_sound.intoRustString()
    pixel_offset = world.pixel_offset
    line_spacing = world.line_spacing
    output_font = world.output_font.intoRustString()
    use_default_output_font = world.use_default_output_font
    show_bold = world.show_bold
    show_italic = world.show_italic
    show_underline = world.show_underline
    new_activity_sound = world.new_activity_sound.intoRustString()
    max_output_lines = world.max_output_lines
    wrap_column = world.wrap_column
    line_information = world.line_information
    start_paused = world.start_paused
    auto_pause = world.auto_pause
    unpause_on_send = world.unpause_on_send
    flash_taskbar_icon = world.flash_taskbar_icon
    disable_compression = world.disable_compression
    indent_paras = world.indent_paras
    naws = world.naws
    carriage_return_clears_line = world.carriage_return_clears_line
    utf_8 = world.utf_8
    auto_wrap_window_width = world.auto_wrap_window_width
    show_connect_disconnect = world.show_connect_disconnect
    copy_selection_to_clipboard = world.copy_selection_to_clipboard
    auto_copy_to_clipboard_in_html = world.auto_copy_to_clipboard_in_html
    convert_ga_to_newline = world.convert_ga_to_newline
    terminal_identification = world.terminal_identification.intoRustString()
    use_mxp = world.use_mxp
    detect_pueblo = world.detect_pueblo
    hyperlink_color = HexColor(world.hyperlink_color)
    use_custom_link_color = world.use_custom_link_color
    mud_can_change_link_color = world.mud_can_change_link_color
    underline_hyperlinks = world.underline_hyperlinks
    mud_can_remove_underline = world.mud_can_remove_underline
    hyperlink_adds_to_command_history = world.hyperlink_adds_to_command_history
    echo_hyperlink_in_output_window = world.echo_hyperlink_in_output_window
    ignore_mxp_color_changes = world.ignore_mxp_color_changes
    send_mxp_afk_response = world.send_mxp_afk_response
    mud_can_change_options = world.mud_can_change_options
    use_default_colors = world.use_default_colors
    ansi_colors = intoVec(world.ansi_colors, by: HexColor.init)
    custom_names = intoVec(world.custom_names, by: { $0.intoRustString() })
    custom_colors = intoVec(world.custom_colors, by: ColorPair.init)
    triggers = intoVec(world.triggers, by: Trigger.init)
    enable_triggers = world.enable_triggers
    enable_trigger_sounds = world.enable_trigger_sounds
    treeview_triggers = world.treeview_triggers
    display_my_input = world.display_my_input
    echo_colors = ColorPair(world.echo_colors)
    enable_speed_walk = world.enable_speed_walk
    speed_walk_prefix = world.speed_walk_prefix.intoRustString()
    speed_walk_filler = world.speed_walk_filler.intoRustString()
    speed_walk_delay = world.speed_walk_delay
    enable_command_stack = world.enable_command_stack
    command_stack_character = world.command_stack_character.intoRustString()
    input_colors = ColorPair(world.input_colors)
    input_font = world.input_font.intoRustString()
    use_default_input_font = world.use_default_input_font
    enable_spam_prevention = world.enable_spam_prevention
    spam_line_count = world.spam_line_count
    spam_message = world.spam_message.intoRustString()
    auto_repeat = world.auto_repeat
    lower_case_tab_completion = world.lower_case_tab_completion
    translate_german = world.translate_german
    translate_backslash_sequences = world.translate_backslash_sequences
    keep_commands_on_same_line = world.keep_commands_on_same_line
    no_echo_off = world.no_echo_off
    tab_completion_lines = world.tab_completion_lines
    tab_completion_space = world.tab_completion_space
    double_click_inserts = world.double_click_inserts
    double_click_sends = world.double_click_sends
    escape_deletes_input = world.escape_deletes_input
    save_deleted_command = world.save_deleted_command
    confirm_before_replacing_typing = world.confirm_before_replacing_typing
    arrow_keys_wrap = world.arrow_keys_wrap
    arrows_change_history = world.arrows_change_history
    arrow_recalls_partial = world.arrow_recalls_partial
    alt_arrow_recalls_partial = world.alt_arrow_recalls_partial
    ctrl_z_goes_to_end_of_buffer = world.ctrl_z_goes_to_end_of_buffer
    ctrl_p_goes_to_previous_command = world.ctrl_p_goes_to_previous_command
    ctrl_n_goes_to_next_command = world.ctrl_n_goes_to_next_command
    history_lines = world.history_lines
    aliases = intoVec(world.aliases, by: Alias.init)
    enable_aliases = world.enable_aliases
    treeview_aliases = world.treeview_aliases
    keypad_enable = world.keypad_enable
    keypad_shortcuts = mappingToRust(world.keypad_shortcuts)
    enable_auto_say = world.enable_auto_say
    autosay_exclude_non_alpha = world.autosay_exclude_non_alpha
    autosay_exclude_macros = world.autosay_exclude_macros
    auto_say_override_prefix = world.auto_say_override_prefix.intoRustString()
    auto_say_string = world.auto_say_string.intoRustString()
    re_evaluate_auto_say = world.re_evaluate_auto_say
    paste_line_preamble = world.paste_line_preamble.intoRustString()
    paste_line_postamble = world.paste_line_postamble.intoRustString()
    paste_delay = world.paste_delay
    paste_delay_per_lines = world.paste_delay_per_lines
    paste_commented_softcode = world.paste_commented_softcode
    paste_echo = world.paste_echo
    confirm_on_paste = world.confirm_on_paste
    send_line_preamble = world.send_line_preamble.intoRustString()
    send_line_postamble = world.send_line_postamble.intoRustString()
    send_delay = world.send_delay
    send_delay_per_lines = world.send_delay_per_lines
    send_commented_softcode = world.send_commented_softcode
    send_echo = world.send_echo
    confirm_on_send = world.confirm_on_send
    world_script = world.world_script.intoRustString()
    script_prefix = world.script_prefix.intoRustString()
    enable_scripts = world.enable_scripts
    warn_if_scripting_inactive = world.warn_if_scripting_inactive
    edit_script_with_notepad = world.edit_script_with_notepad
    script_editor = world.script_editor.intoRustString()
    script_reload_option = world.script_reload_option
    script_errors_to_output_window = world.script_errors_to_output_window
    note_text_color = HexColor(world.note_text_color)
    plugins = intoVec(world.plugins, by: { $0.intoRustString() })
  }
}
