---@meta

---@alias BooleanString
---| "y" # true
---| "Y" # true
---| "1" # true
---| "n" # false
---| "N" # false
---| "0" # false

---@alias BooleanInt
---| 0 # false
---| 1 # true

---@alias ClientOptionBool
---| "accept_chat_connections"
---| "alternative_inverse"
---| "alt_arrow_recalls_partial"
---| "always_record_command_history"
---| "arrows_change_history"
---| "arrow_keys_wrap"
---| "arrow_recalls_partial"
---| "autosay_exclude_macros"
---| "autosay_exclude_non_alpha"
---| "auto_allow_files"
---| "auto_allow_snooping"
---| "auto_copy_to_clipboard_in_html"
---| "auto_pause"
---| "keep_pause_at_bottom"
---| "auto_repeat"
---| "auto_resize_command_window"
---| "auto_wrap_window_width"
---| "chat_sumchecks_to_sender"
---| "confirm_before_replacing_typing"
---| "confirm_on_paste"
---| "confirm_on_send"
---| "copy_selection_to_clipboard"
---| "ctrl_n_goes_to_next_command"
---| "ctrl_p_goes_to_previous_command"
---| "ctrl_z_goes_to_end_of_buffer"
---| "ctrl_backspace_deletes_last_word"
---| "custom_16_is_default_colour"
---| "default_trigger_regexp"
---| "default_trigger_expand_variables"
---| "default_trigger_keep_evaluating"
---| "default_trigger_ignore_case"
---| "default_alias_regexp"
---| "default_alias_expand_variables"
---| "default_alias_keep_evaluating"
---| "default_alias_ignore_case"
---| "detect_pueblo"
---| "do_not_add_macros_to_command_history"
---| "do_not_show_outstanding_lines"
---| "do_not_translate_iac_to_iac_iac"
---| "double_click_inserts"
---| "double_click_sends"
---| "edit_script_with_notepad"
---| "enable_auto_say"
---| "enable_beeps"
---| "enable_spam_prevention"
---| "escape_deletes_input"
---| "flash_taskbar_icon"
---| "ignore_chat_colours"
---| "input_font_italic"
---| "lower_case_tab_completion"
---| "map_failure_regexp"
---| "mud_can_change_options"
---| "omit_date_from_save_files"
---| "paste_commented_softcode"
---| "paste_echo"
---| "play_sounds_in_background"
---| "pixel_offset"
---| "proxy_type"
---| "re_evaluate_auto_say"
---| "save_deleted_command"
---| "send_echo"
---| "send_double_ff_for_hex_ff"
---| "send_file_commented_softcode"
---| "send_mxp_afk_response"
---| "show_connect_disconnect"
---| "show_grid_lines_in_list_views"
---| "spell_check_on_send"
---| "start_paused"
---| "tab_completion_space"
---| "translate_backslash_sequences"
---| "translate_german"
---| "treeview_triggers"
---| "treeview_aliases"
---| "treeview_timers"
---| "unpause_on_send"
---| "use_default_aliases"
---| "use_default_input_font"
---| "use_default_macros"
---| "use_default_output_font"
---| "use_default_timers"
---| "use_default_triggers"
---| "validate_incoming_chat_calls"
---| "warn_if_scripting_inactive"
---| "wrap"
---| "wrap_input"

---@alias ClientOptionInt
---| "auto_resize_minimum_lines"
---| "auto_resize_maximum_lines"
---| "chat_foreground_colour"
---| "chat_background_colour"
---| "chat_max_lines_per_message"
---| "chat_max_bytes_per_message"
---| "chat_port"
---| "default_trigger_send_to"
---| "default_trigger_sequence"
---| "default_alias_send_to"
---| "default_alias_sequence"
---| "default_timer_send_to"
---| "echo_colour"
---| "fade_output_buffer_after_seconds"
---| "fade_output_opacity_percent"
---| "fade_output_seconds"
---| "history_lines"
---| "input_background_colour"
---| "input_font_height"
---| "input_font_weight"
---| "input_font_charset"
---| "input_text_colour"
---| "line_spacing"
---| "max_output_lines"
---| "output_font_height"
---| "output_font_weight"
---| "output_font_charset"
---| "paste_delay"
---| "paste_delay_per_lines"
---| "send_file_delay"
---| "send_file_delay_per_lines"
---| "send_keep_alives"
---| "spam_line_count"
---| "tab_completion_lines"
---| "timestamp_input_text_colour"
---| "timestamp_notes_text_colour"
---| "timestamp_output_text_colour"
---| "timestamp_input_back_colour"
---| "timestamp_notes_back_colour"
---| "timestamp_output_back_colour"
---| "tool_tip_visible_time"
---| "tool_tip_start_time"
---| "wrap_column"

---@alias ClientOptionString
---| "auto_say_override_prefix"
---| "auto_say_string"
---| "beep_sound"
---| "chat_file_save_directory"
---| "chat_name"
---| "chat_message_prefix"
---| "editor_window_name"
---| "filter_aliases"
---| "filter_timers"
---| "filter_triggers"
---| "filter_variables"
---| "id"
---| "input_font_name"
---| "mapping_failure"
---| "notes"
---| "on_mxp_close_tag"
---| "on_mxp_error"
---| "on_mxp_open_tag"
---| "on_mxp_set_variable"
---| "on_mxp_start"
---| "on_mxp_stop"
---| "on_world_close"
---| "on_world_save"
---| "on_world_connect"
---| "on_world_disconnect"
---| "on_world_get_focus"
---| "on_world_lose_focus"
---| "on_world_open"
---| "output_font_name"
---| "paste_line_postamble"
---| "paste_line_preamble"
---| "paste_postamble"
---| "paste_preamble"
---| "recall_line_preamble"
---| "script_editor"
---| "script_editor_argument"
---| "script_filename"
---| "script_language"
---| "script_prefix"
---| "send_to_world_file_postamble"
---| "send_to_world_file_preamble"
---| "send_to_world_line_postamble"
---| "send_to_world_line_preamble"
---| "spam_message"
---| "speed_walk_filler"
---| "speed_walk_prefix"
---| "tab_completion_defaults"
---| "timestamp_input"
---| "timestamp_notes"
---| "timestamp_output"


---@alias WorldOptionBool
---| "carriage_return_clears_line"
---| "command_stack_delay"
---| "convert_ga_to_newline"
---| "disable_compression"
---| "display_my_input"
---| "echo_hotkey_in_output_window"
---| "echo_hyperlink_in_output_window"
---| "enable_aliases"
---| "enable_command_stack"
---| "enable_scripts"
---| "enable_speed_walk"
---| "enable_timers"
---| "enable_trigger_sounds"
---| "enable_triggers"
---| "hotkey_adds_to_command_history"
---| "hyperlink_adds_to_command_history"
---| "ignore_mxp_colour_changes"
---| "keep_commands_on_same_line"
---| "keypad_enable"
---| "line_information"
---| "log_html"
---| "log_input"
---| "log_in_colour"
---| "log_notes"
---| "log_output"
---| "log_raw"
---| "log_script_errors"
---| "mud_can_change_link_colour"
---| "mud_can_remove_underline"
---| "naws"
---| "no_echo_off"
---| "save_world_automatically"
---| "script_errors_to_output_window"
---| "show_bold"
---| "show_italic"
---| "show_underline"
---| "underline_hyperlinks"
---| "use_custom_link_colour"
---| "use_default_colours"
---| "utf_8"
---| "write_world_name_to_log"

---@alias WorldOptionInt
---| "connect_method" # 0 - 3
---| "hyperlink_colour"
---| "indent_paras"
---| "mxp_debug_level" # 0 - 4
---| "note_background_colour"
---| "note_text_colour"
---| "port"
---| "proxy_port"
---| "script_reload_option" # 0 - 2
---| "speed_walk_delay"
---| "use_mxp" # 0 - 3

---@alias WorldOptionString
---| "auto_log_file_name"
---| "command_stack_character"
---| "connect_text"
---| "log_file_postamble"
---| "log_file_preamble"
---| "log_line_postamble_input"
---| "log_line_postamble_notes"
---| "log_line_postamble_output"
---| "log_line_preamble_input"
---| "log_line_preamble_notes"
---| "log_line_preamble_output"
---| "name"
---| "new_activity_sound"
---| "player"
---| "proxy_username"
---| "site"
---| "terminal_identification"

---@alias WorldOptionStringSecure
---| "password"
---| "proxy_password"
---| "proxy_server"


---@alias CommonOptionBool
---| "enabled" # enabled
---| "omit_from_log" # omit from log file
---| "omit_from_output" # omit sent text from the output window
---| "one_shot" # deleted after firing

---@alias CommonOptionInt
---| "user" # -2147483647 to 2147483647 - user-defined number

---@alias CommonOptionString
---| "group" # group name
---| "name" # name/label
---| "script" # name of function to call
---| "send" # what to send
---| "variable" # name of variable to send to


---@alias MatcherOptionBool
---| CommonOptionBool
---| "expand_variables" # expand variables (like @target)
---| "ignore_case" # caseless matching
---| "keep_evaluating" # evaluate next in sequence
---| "regexp" # regular expression
---| "repeat" # repeatedly evaluate on same line

---@alias MatcherOptionInt
---| CommonOptionInt
---| "sequence" # 0 - 10000 - sequence number in which to check - lower first

---@alias MatcherOptionString
---| CommonOptionString
---| "match" # what to match


---@alias AliasOptionBool
---| MatcherOptionBool
---| "echo_alias" # echo alias itself to output window
---| "menu" # add alias to menu that appears if you LH click
---| "omit_from_command_history" # omit alias from the command history

---@alias AliasOptionInt MatcherOptionInt

---@alias AliasOptionString MatcherOptionString


---@alias TimerOptionBool
---| "active_closed" # timer is active when world closed
---| "at_time" # y=fire *at* time of day, otherwise every interval

---@alias TimerOptionInt
---| CommonOptionInt
---| "hour" # hour to fire at (or every hour)
---| "minute" # minute to fire at (or every minute)
---| "second" # hour to fire at (or every second)

---@alias TimerOptionString CommonOptionString


---@alias TriggerOptionBool
---| MatcherOptionBool
---| "lowercase_wildcard" # make matching wildcards lower-case
---| "multi_line" # multi-line trigger
---| "sound_if_inactive" # play sound even if world inactive

---@alias TriggerOptionInt
---| MatcherOptionInt
---| "clipboard_arg" # 0 - 10 - which wildcard to copy to the clipboard
---| "colour_change_type" # 0=both, 1=foreground, 2=background
---| "custom_colour" # 0=no change, 1 - 16, 17=other
---| "lines_to_match" # 0 - 200 - how many lines to match for multi-line triggers
---| "new_style" # (style(s) to change to: (bold (1) / underline (2) / italic (4) ) )
---| "other_back_colour" # BBGGRR colour code of colour to change background
---| "other_text_colour" # BBGGRR colour code of colour to change foreground

---@alias TriggerOptionString
---| MatcherOptionString
---| "sound" # sound file name to play
---| "other_back_colour" # hex code or name of colour to change background
---| "other_text_colour" # hex code or name of colour to change foreground


---Gets the current value of a *boolean* alias option. If called from inside the plugin, the aliases for the current plugin are used. If not, the "global" aliases are used.
---
---For a detailed description about the meanings of the fields, see [`AddAlias`](lua://AddAlias).
---@param aliasName string The name of an existing alias.
---@param optionName AliasOptionBool Option name. See below.
---@return boolean|nil option Alias option value, or nil if there is no such alias.
---
---@see SetAliasOption - setter.
---@see GetPluginAliasOption - get option for alias from another plugin.
function GetAliasOption(aliasName, optionName) end

---Gets the current value of a *boolean* alias option. If called from inside the plugin, the aliases for the current plugin are used. If not, the "global" aliases are used.
---
---For a detailed description about the meanings of the fields, see [`AddAlias`](lua://AddAlias).
---@param aliasName string The name of an existing alias.
---@param optionName AliasOptionInt Option name. See below.
---@return integer|nil option Alias option value, or nil if there is no such alias.
---
---@see SetAliasOption - setter.
---@see GetPluginAliasOption - get option for alias from another plugin.
function GetAliasOption(aliasName, optionName) end

---Gets the current send-to location option of an alias. If called from inside the plugin, the aliases for the current plugin are used. If not, the "global" aliases are used.
---
---For a detailed description about the meanings of the fields, see [`AddAlias`](lua://AddAlias).
---@param aliasName string The name of an existing alias.
---@param optionName
---| "send_to" # Send-to location
---@return sendto|nil option Alias option value, or nil if there is no such alias.
---
---@see SetAliasOption - setter.
---@see GetPluginAliasOption - get option for alias from another plugin.
function GetAliasOption(aliasName, optionName) end

---Gets the current value of a *string* alias option. If called from inside the plugin, the aliases for the current plugin are used. If not, the "global" aliases are used.
---
---For a detailed description about the meanings of the fields, see [`AddAlias`](lua://AddAlias).
---@param aliasName string The name of an existing alias.
---@param optionName AliasOptionString Option name. See below.
---@return string|nil option Alias option value, or nil if there is no such alias.
---
---@see SetAliasOption - setter.
---@see GetPluginAliasOption - get option for alias from another plugin.
function GetAliasOption(aliasName, optionName) end


---Gets the value of a *string* world option.
---@param optionName WorldOptionString Option name. See below.
---@return string option Option value.
---
---@see SetAlphaOption - setter.
---@see GetCurrentValue - generic version.
---@see GetAlphaOptionList - list all available *optionName*s.
function GetAlphaOption(optionName) end

---Gets the value of a secure *string* world option. Secure options cannot be accessed by plugins.
---@param optionName WorldOptionStringSecure Option name. See below.
---@return string|nil option Option value, or nil if this function is called from a plugin.
---
---@see SetAlphaOption - setter.
---@see GetCurrentValue - generic version.
---@see GetAlphaOptionList - list all available *optionName*s.
function GetAlphaOption(optionName) end

--[[
---Gets the value of an unimplemented string configuration option.
---@param optionName ClientOptionString
---@return nil option
function GetAlphaOption(optionName) end
]]

---Gets the list of world string options.
---@return string[] optionNames
---
---@see GetOptionList
---@see GetAlphaOption
---@see SetAlphaOption
function GetAlphaOptionList() end


---Gets the current value of a *boolean* world option.
---@param optionName WorldOptionBool Option name. See below.
---@return BooleanInt option Option value.
---
---@see SetOption - setter.
---@see GetOption - type-specialized version.
---@see GetOptionList - list numeric (and boolean) options.
function GetCurrentValue(optionName) end

---Gets the current value of an *integer* world option.
---@param optionName WorldOptionInt Option name. See below.
---@return integer option Option value.
---
---@see SetOption - setter.
---@see GetOption - type-specialized version.
---@see GetOptionList - list numeric (and boolean) options.
function GetCurrentValue(optionName) end

---Gets the current value of a *string* world option.
---@param optionName WorldOptionString Option name. See below.
---@return string option Option value.
---
---@see SetAlphaOption - setter.
---@see GetAlphaOption - type-specialized version.
---@see GetAlphaOptionList - list string options.
function GetCurrentValue(optionName) end

---Gets the value of a secure *string* world option. Secure options cannot be accessed by plugins.
---@param optionName WorldOptionStringSecure Option name. See below.
---@return string|nil option Option value, or nil if this function is called from a plugin.
---
---@see SetAlphaOption - setter.
---@see GetAlphaOption - type-specialized version.
---@see GetAlphaOptionList - list string options.
function GetCurrentValue(optionName) end

--[[
---Gets the current value of an unimplemented configuration option…
---@param optionName ClientOptionBool|ClientOptionInt|ClientOptionString
---@return nil
function GetCurrentValue(optionName) end
]]

---Gets the current value of a *boolean* world option.
---@param optionName WorldOptionBool Option name. See below.
---@return BooleanInt option Option value.
---
---@see SetOption - setter.
---@see GetCurrentValue - generic version.
---@see GetOptionList - list all available *optionName*s.
function GetOption(optionName) end

---Gets the current value of an *integer* world option.
---@param optionName WorldOptionInt Option name. See below.
---@return integer option Option value.
---
---@see SetOption - setter.
---@see GetCurrentValue - generic version.
---@see GetOptionList - list all available *optionName*s.
function GetOption(optionName) end

--[[
---Gets the current value of an unimplemented configuration option.
---@param optionName ClientOptionBool|ClientOptionInt
---@return nil
function GetOption(optionName) end
]]

---Gets the list of world numeric options.
---@return string[] optionNames
---
---@see GetAlphaOptionList
---@see GetOption
---@see SetOption
function GetOptionList() end


---Gets the current value of a *boolean* alias option for an alias from another plugin.
---
---For a detailed description about the meanings of the fields, see [`AddAlias`](lua://AddAlias).
---@param pluginID string ID of plugin that contains the alias.
---@param aliasName string The name of an existing alias.
---@param optionName AliasOptionBool Option name. See below.
---@return boolean|nil option Alias option value, or nil if the plugin does not exist or does not have an alias by that name.
---
---@see GetAliasOption - get option for alias from current plugin.
function GetPluginAliasOption(pluginID, aliasName, optionName) end

---Gets the current value of a *boolean* alias option for an alias from another plugin.
---
---For a detailed description about the meanings of the fields, see [`AddAlias`](lua://AddAlias).
---@param pluginID string ID of plugin that contains the alias.
---@param aliasName string The name of an existing alias.
---@param optionName AliasOptionInt Option name. See below.
---@return integer|nil option Alias option value, or nil if the plugin does not exist or does not have an alias by that name.
---
---@see GetAliasOption - get option for alias from current plugin.
function GetPluginAliasOption(pluginID, aliasName, optionName) end

---Gets the current send-to location option for an alias from another plugin.
---
---For a detailed description about the meanings of the fields, see [`AddAlias`](lua://AddAlias).
---@param pluginID string ID of plugin that contains the alias.
---@param aliasName string The name of an existing alias.
---@param optionName
---| "send_to" # Send-to location
---@return sendto|nil option Alias option value, or nil if the plugin does not exist or does not have an alias by that name.
---
---@see GetAliasOption - get option for alias from current plugin.
function GetPluginAliasOption(pluginID, aliasName, optionName) end

---Gets the current value of a *string* alias option for an alias from another plugin.
---
---For a detailed description about the meanings of the fields, see [`AddAlias`](lua://AddAlias).
---@param pluginID string ID of plugin that contains the alias.
---@param aliasName string The name of an existing alias.
---@param optionName AliasOptionString Option name. See below.
---@return string|nil option Alias option value, or nil if the plugin does not exist or does not have an alias by that name.
---
---@see GetAliasOption - get option for alias from current plugin.
function GetPluginAliasOption(pluginID, aliasName, optionName) end


---Gets the current value of a *boolean* timer option for a timer from another plugin.
---
---For a detailed description about the meanings of the fields, see [`AddTimer`](lua://AddTimer).
---@param pluginID string ID of plugin that contains the timer.
---@param timerName string The name of an existing timer.
---@param optionName TimerOptionBool Option name. See below.
---@return boolean|nil option Timer option value, or nil if the plugin does not exist or does not have a timer by that name.
---
---@see GetTimerOption - get option for timer from current plugin.
function GetPluginTimerOption(pluginID, timerName, optionName) end

---Gets the current value of an *integer* timer option for a timer from another plugin.
---
---For a detailed description about the meanings of the fields, see [`AddTimer`](lua://AddTimer).
---@param pluginID string ID of plugin that contains the timer.
---@param timerName string The name of an existing timer.
---@param optionName TimerOptionInt Option name. See below.
---@return integer|nil option Timer option value, or nil if the plugin does not exist or does not have a timer by that name.
---
---@see GetTimerOption - get option for timer from current plugin.
function GetPluginTimerOption(pluginID, timerName, optionName) end

---Gets the current send-to location of a timer option for a timer from another plugin.
---
---For a detailed description about the meanings of the fields, see [`AddTimer`](lua://AddTimer).
---@param pluginID string ID of plugin that contains the timer.
---@param timerName string The name of an existing timer.
---@param optionName
---| "send_to" # Send-to location
---@return sendto|nil option Timer option value, or nil if the plugin does not exist or does not have a timer by that name.
---
---@see GetTimerOption - get option for timer from current plugin.
function GetPluginTimerOption(pluginID, timerName, optionName) end

---Gets the current value of a *string* timer option for a timer from another plugin.
---
---For a detailed description about the meanings of the fields, see [`AddTimer`](lua://AddTimer).
---@param pluginID string ID of plugin that contains the timer.
---@param timerName string The name of an existing timer.
---@param optionName TimerOptionString Option name. See below.
---@return string|nil option Timer option value, or nil if the plugin does not exist or does not have a timer by that name.
---
---@see GetTimerOption - get option for timer from current plugin.
function GetPluginTimerOption(pluginID, timerName, optionName) end


---Gets the current value of a *boolean* trigger option for a trigger from another plugin.
---
---For a detailed description about the meanings of the fields, see [`AddTrigger`](lua://AddTrigger).
---@param pluginID string ID of plugin that contains the trigger.
---@param triggerName string The name of an existing trigger.
---@param optionName TriggerOptionBool Option name. See below.
---@return boolean|nil option Trigger option value, or nil if the plugin does not exist or does not have a trigger by that name.
---
---@see GetTriggerOption - get option for trigger from current plugin.
function GetPluginTriggerOption(pluginID, triggerName, optionName) end

---Gets the current value of an *integer* trigger option for a trigger from another plugin.
---
---For a detailed description about the meanings of the fields, see [`AddTrigger`](lua://AddTrigger).
---@param pluginID string ID of plugin that contains the trigger.
---@param triggerName string The name of an existing trigger.
---@param optionName TriggerOptionInt Option name. See below.
---@return integer|nil option Trigger option value, or nil if the plugin does not exist or does not have a trigger by that name.
---
---@see GetTriggerOption - get option for trigger from current plugin.
function GetPluginTriggerOption(pluginID, triggerName, optionName) end

---Gets the current send-to location of a trigger for a trigger from another plugin.
---
---For a detailed description about the meanings of the fields, see [`AddTrigger`](lua://AddTrigger).
---@param pluginID string ID of plugin that contains the trigger.
---@param triggerName string The name of an existing triogger.
---@param optionName
---| "send_to" # Send-to location
---@return sendto|nil option Trigger option value, or nil if the plugin does not exist or does not have a trigger by that name.
---
---@see GetTriggerOption - get option for trigger from current plugin.
function GetPluginTriggerOption(pluginID, triggerName, optionName) end

---Gets the current value of a *string* trigger option for a trigger from another plugin.
---
---For a detailed description about the meanings of the fields, see [`AddTrigger`](lua://AddTrigger).
---@param pluginID string ID of plugin that contains the trigger.
---@param triggerName string The name of an existing trigger.
---@param optionName TriggerOptionString Option name. See below.
---@return string|nil option Trigger option value, or nil if the plugin does not exist or does not have a trigger by that name.
---
---@see GetTriggerOption - get option for trigger from current plugin.
function GetPluginTriggerOption(pluginID, triggerName, optionName) end


---Gets the current value of a *boolean* timer option. If called from inside the plugin, the timers for the current plugin are used. If not, the "global" timers are used.
---
---For a detailed description about the meanings of the fields, see [`AddTimer`](lua://AddTimer).
---@param timerName string The name of an existing timer.
---@param optionName TimerOptionBool Option name. See below.
---@return boolean|nil option Timer option value, or nil if the plugin does not exist or does not have a trigger by that name.
---
---@see SetTimerOption - setter.
---@see GetPluginTimerOption - get option for timer from another plugin.
function GetTimerOption(timerName, optionName) end

---Gets the current value of an *integer* timer option. If called from inside the plugin, the timers for the current plugin are used. If not, the "global" timers are used.
---
---For a detailed description about the meanings of the fields, see [`AddTimer`](lua://AddTimer).
---@param timerName string The name of an existing timer.
---@param optionName TimerOptionInt Option name. See below.
---@return integer|nil option Timer option value, or nil if the plugin does not exist or does not have a trigger by that name.
---
---@see SetTimerOption - setter.
---@see GetPluginTimerOption - get option for timer from another plugin.
function GetTimerOption(timerName, optionName) end

---Gets the current send-to location of a timer option. If called from inside the plugin, the timers for the current plugin are used. If not, the "global" timers are used.
---
---For a detailed description about the meanings of the fields, see [`AddTimer`](lua://AddTimer).
---@param timerName string The name of an existing timer.
---@param optionName
---| "send_to" # Send-to location
---@return sendto|nil option Timer option value, or nil if the plugin does not exist or does not have a trigger by that name.
---
---@see SetTimerOption - setter.
---@see GetPluginTimerOption - get option for timer from another plugin.
function GetTimerOption(timerName, optionName) end

---Gets the current value of a *string* timer option. If called from inside the plugin, the timers for the current plugin are used. If not, the "global" timers are used.
---
---For a detailed description about the meanings of the fields, see [`AddTimer`](lua://AddTimer).
---@param timerName string The name of an existing timer.
---@param optionName TimerOptionString Option name. See below.
---@return string|nil option Timer option value, or nil if the plugin does not exist or does not have a trigger by that name.
---
---@see SetTimerOption - setter.
---@see GetPluginTimerOption - get option for timer from another plugin.
function GetTimerOption(timerName, optionName) end


---Gets the current value of a *boolean* trigger option. If called from inside the plugin, the triggers for the current plugin are used. If not, the "global" triggers are used.
---
---For a detailed description about the meanings of the fields, see [`AddTrigger`](lua://AddTrigger).
---@param triggerName string The name of an existing trigger.
---@param optionName TriggerOptionBool Option name. See below.
---@return boolean|nil option Trigger option value, or nil if there is no such trigger.
---
---@see SetTriggerOption - setter.
---@see GetPluginTriggerOption - get option for trigger from another plugin.
function GetTriggerOption(triggerName, optionName) end

---Gets the current value of an *integer* trigger option. If called from inside the plugin, the triggers for the current plugin are used. If not, the "global" triggers are used.
---
---For a detailed description about the meanings of the fields, see [`AddTrigger`](lua://AddTrigger).
---@param triggerName string The name of an existing trigger.
---@param optionName TriggerOptionInt Option name. See below.
---@return integer|nil option Trigger option value, or nil if there is no such trigger.
---
---@see SetTriggerOption - setter.
---@see GetPluginTriggerOption - get option for trigger from another plugin.
function GetTriggerOption(triggerName, optionName) end

---Gets the current send-to location of a trigger. If called from inside the plugin, the triggers for the current plugin are used. If not, the "global" triggers are used.
---
---For a detailed description about the meanings of the fields, see [`AddTrigger`](lua://AddTrigger).
---@param triggerName string The name of an existing triogger.
---@param optionName
---| "send_to" # Send-to location
---@return sendto|nil option Trigger option value, or nil if there is no such trigger.
---
---@see SetTriggerOption - setter.
---@see GetPluginTriggerOption - get option for trigger from another plugin.
function GetTriggerOption(triggerName, optionName) end

---Gets the current value of a *string* trigger option. If called from inside the plugin, the triggers for the current plugin are used. If not, the "global" triggers are used.
---
---For a detailed description about the meanings of the fields, see [`AddTrigger`](lua://AddTrigger).
---@param triggerName string The name of an existing trigger.
---@param optionName TriggerOptionString Option name. See below.
---@return string|nil option Trigger option value, or nil if there is no such trigger.
---
---@see SetTriggerOption - setter.
---@see GetPluginTriggerOption - get option for trigger from another plugin.
function GetTriggerOption(triggerName, optionName) end


---Sets the current value of a *boolean* alias option.
---
---If `SetAliasOption` is called from within a plugin, the aliases for the current plugin are used, not the "global" aliases.
---@param aliasName string The name of an existing alias.
---@param optionName AliasOptionBool Option name. See below.
---@param value boolean|BooleanString Option value.
---@return error_code code #
---`error_code.eInvalidObjectLabel`: The alias name is not valid.\
---`error_code.eAliasCannotBeEmpty`: The "match_text" cannot be empty.\
---`error_code.eScriptNameNotLocated`: The script name cannot be located in the script file.\
---`error_code.eBadRegularExpression`: The regular expression could not be evaluated.\
---`error_code.ePluginCannotSetOption`: The option is marked as non-settable (in a plugin or not).\
---`error_code.eOK`: Set OK
---
---@see GetAliasOption - getter.
function SetAliasOption(aliasName, optionName, value) end

---Sets the current value of an *integer* alias option.
---
---If `SetAliasOption` is called from within a plugin, the aliases for the current plugin are used, not the "global" aliases.
---
---The supplied string will be converted to a number. It is an error if the string does not consist of digits (0 to 9). For the "user" option an optional leading sign is accepted.
---@param aliasName string The name of an existing alias.
---@param optionName AliasOptionInt Option name. See below.
---@param value integer|string Option value.
---@return error_code code #
---`error_code.eInvalidObjectLabel`: The alias name is not valid.\
---`error_code.eAliasCannotBeEmpty`: The "match_text" cannot be empty.\
---`error_code.eScriptNameNotLocated`: The script name cannot be located in the script file.\
---`error_code.eBadRegularExpression`: The regular expression could not be evaluated.\
---`error_code.ePluginCannotSetOption`: The option is marked as non-settable (in a plugin or not).\
---`error_code.eOK`: Set OK
---
---@see GetAliasOption - getter.
function SetAliasOption(aliasName, optionName, value) end

---Sets the current send-to location of an alias.
---
---If `SetAliasOption` is called from within a plugin, the aliases for the current plugin are used, not the "global" aliases.
---@param aliasName string The name of an existing alias.
---@param optionName
---| "send_to" # Send-to location
---@param value sendto|string Option value.
---@return error_code code #
---`error_code.eInvalidObjectLabel`: The alias name is not valid.\
---`error_code.eAliasCannotBeEmpty`: The "match_text" cannot be empty.\
---`error_code.eScriptNameNotLocated`: The script name cannot be located in the script file.\
---`error_code.eBadRegularExpression`: The regular expression could not be evaluated.\
---`error_code.ePluginCannotSetOption`: The option is marked as non-settable (in a plugin or not).\
---`error_code.eOK`: Set OK
---
---@see GetAliasOption - getter.
function SetAliasOption(aliasName, optionName, value) end

---Sets the current value of a *string* alias option.
---
---If `SetAliasOption` is called from within a plugin, the aliases for the current plugin are used, not the "global" aliases.
---@param aliasName string The name of an existing alias.
---@param optionName AliasOptionString Option name. See below.
---@param value string Option value.
---@return error_code code #
---`error_code.eInvalidObjectLabel`: The alias name is not valid.\
---`error_code.eAliasCannotBeEmpty`: The "match_text" cannot be empty.\
---`error_code.eScriptNameNotLocated`: The script name cannot be located in the script file.\
---`error_code.eBadRegularExpression`: The regular expression could not be evaluated.\
---`error_code.ePluginCannotSetOption`: The option is marked as non-settable (in a plugin or not).\
---`error_code.eOK`: Set OK
---
---@see GetAliasOption - getter.
function SetAliasOption(aliasName, optionName, value) end


---Sets the value of a *string* world option.
---@param optionName WorldOptionString Option name. See below.
---@param value string Option value.
---@return error_code code #
---`error_code.eUnknownOption`: Option name not recognised.\
---`error_code.eOK`: Set OK.
---
---@see GetAlphaOption - setter.
---@see GetAlphaOptionList
---@see SetOption
function SetAlphaOption(optionName, value) end

--[[
---Sets the value of a *string* unimplemented configuration option.
---@param optionName ClientOptionString Option name. See below.
---@param value string Option value.
---@return error_code code #
---`error_code.eUnknownOption`: option name not recognised\
---`error_code.eOK`: set OK
function SetAlphaOption(optionName, value) end
]]

---Sets the value of a *boolean* world option.
---@param optionName WorldOptionBool|ClientOptionBool Option name. See below.
---@param value boolean|BooleanInt Option value.
---@return error_code code #
---`error_code.eUnknownOption`: Option name not recognised.\
---`error_code.eOptionOutOfRange`: Option value out of range.\
---`error_code.ePluginCannotSetOption`: The option is marked as non-settable (in a plugin or not).\
---`error_code.eOK`: Set OK.
---
---@see GetOption - setter.
---@see GetOptionList
---@see SetAlphaOption
function SetOption(optionName, value) end

---Sets the value of an *integer* world option.
---@param optionName WorldOptionInt|ClientOptionInt Option name. See below.
---@param value integer Option value.
---@return error_code code #
---`error_code.eUnknownOption`: Option name not recognised.\
---`error_code.eOptionOutOfRange`: Option value out of range.\
---`error_code.ePluginCannotSetOption`: The option is marked as non-settable (in a plugin or not).\
---`error_code.eOK`: Set OK.
---
---@see GetOption - setter.
---@see GetOptionList
---@see SetAlphaOption
function SetOption(optionName, value) end


---Sets the current value of a *boolean* timer option.
---
---If `SetTimerOption` is called from within a plugin, the timers for the current plugin are used, not the "global" MUSHclient timers.
---@param timerName string The name of an existing timer.
---@param optionName TimerOptionBool Option name. See below.
---@param value boolean|BooleanString Option value.
---@return error_code code #
---`error_code.eInvalidObjectLabel`: The timer name is not valid.\
---`error_code.eTimeInvalid`: Time is out of range (hours not 0 to 23, minutes/seconds not 0-59).\
---`error_code.eScriptNameNotLocated`: The script name cannot be located in the script file.\
---`error_code.eBadRegularExpression`: The regular expression could not be evaluated.\
---`error_code.ePluginCannotSetOption`: The option is marked as non-settable (in a plugin or not).\
---`error_code.eOK`: Set OK.
---
---@see GetTimerOption - getter.
function SetTimerOption(timerName, optionName, value) end

---Sets the current value of an *integer* timer option.
---
---If `SetTimerOption` is called from within a plugin, the timers for the current plugin are used, not the "global" MUSHclient timers.
---
---The supplied string will be converted to a number. It is an error if the string does not consist of digits (0 to 9). For the "user" option an optional leading sign is accepted.
---@param timerName string The name of an existing timer.
---@param optionName TimerOptionInt Option name. See below.
---@param value integer|string Option value.
---@return error_code code #
---`error_code.eInvalidObjectLabel`: The timer name is not valid.\
---`error_code.eTimeInvalid`: Time is out of range (hours not 0 to 23, minutes/seconds not 0-59).\
---`error_code.eScriptNameNotLocated`: The script name cannot be located in the script file.\
---`error_code.eBadRegularExpression`: The regular expression could not be evaluated.\
---`error_code.ePluginCannotSetOption`: The option is marked as non-settable (in a plugin or not).\
---`error_code.eOK`: Set OK.
---
---@see GetTimerOption - getter.
function SetTimerOption(timerName, optionName, value) end

---Sets the current send-to location of a timer.
---
---If `SetTimerOption` is called from within a plugin, the timers for the current plugin are used, not the "global" MUSHclient timers.
---
---The supplied string will be converted to a number. It is an error if the string does not consist of digits (0 to 9).
---@param timerName string The name of an existing timer.
---@param optionName
---| "send_to" # Send-to location
---@param value sendto|string Option value.
---@return error_code code #
---`error_code.eInvalidObjectLabel`: The timer name is not valid.\
---`error_code.eTimeInvalid`: Time is out of range (hours not 0 to 23, minutes/seconds not 0-59).\
---`error_code.eScriptNameNotLocated`: The script name cannot be located in the script file.\
---`error_code.eBadRegularExpression`: The regular expression could not be evaluated.\
---`error_code.ePluginCannotSetOption`: The option is marked as non-settable (in a plugin or not).\
---`error_code.eOK`: Set OK.
---
---@see GetTimerOption - getter.
function SetTimerOption(timerName, optionName, value) end

---Sets the current value of a *string* timer option.
---
---If `SetTimerOption` is called from within a plugin, the timers for the current plugin are used, not the "global" MUSHclient timers.
---@param timerName string The name of an existing timer.
---@param optionName TimerOptionString Option name. See below.
---@param value string Option value.
---@return error_code code #
---`error_code.eInvalidObjectLabel`: The timer name is not valid.\
---`error_code.eTimeInvalid`: Time is out of range (hours not 0 to 23, minutes/seconds not 0-59).\
---`error_code.eScriptNameNotLocated`: The script name cannot be located in the script file.\
---`error_code.eBadRegularExpression`: The regular expression could not be evaluated.\
---`error_code.ePluginCannotSetOption`: The option is marked as non-settable (in a plugin or not).\
---`error_code.eOK`: Set OK.
---
---@see GetTimerOption - getter.
function SetTimerOption(timerName, optionName, value) end


---Sets the current value of a *boolean* trigger option.
---
---If `SetTriggerOption` is called from within a plugin, the triggers for the current plugin are used, not the "global" MUSHclient timers.
---@param triggerName string The name of an existing trigger.
---@param optionName TriggerOptionBool Option name. See below.
---@param value boolean|BooleanString Option value.
---@return error_code code #
---`error_code.eInvalidObjectLabel`: The trigger name is not valid.\
---`error_code.eTriggerCannotBeEmpty`: The "match_text" cannot be empty.\
---`error_code.eScriptNameNotLocated`: The script name cannot be located in the script file.\
---`error_code.eBadRegularExpression`: The regular expression could not be evaluated.\
---`error_code.ePluginCannotSetOption`: The option is marked as non-settable (in a plugin or not).\
---`error_code.eOK`: Set OK.
---
---@see GetTriggerOption - getter.
function SetTriggerOption(triggerName, optionName, value) end

---Sets the current value of an *integer* trigger option.
---
---If `SetTriggerOption` is called from within a plugin, the triggers for the current plugin are used, not the "global" MUSHclient timers.
---
---The supplied string will be converted to a number. It is an error if the string does not consist of digits (0 to 9). For the "user" option an optional leading sign is accepted.
---@param triggerName string The name of an existing trigger.
---@param optionName TriggerOptionInt Option name. See below.
---@param value integer|string Option value.
---@return error_code code #
---`error_code.eInvalidObjectLabel`: The trigger name is not valid.\
---`error_code.eTriggerCannotBeEmpty`: The "match_text" cannot be empty.\
---`error_code.eScriptNameNotLocated`: The script name cannot be located in the script file.\
---`error_code.eBadRegularExpression`: The regular expression could not be evaluated.\
---`error_code.ePluginCannotSetOption`: The option is marked as non-settable (in a plugin or not).\
---`error_code.eOK`: Set OK.
---
---@see GetTriggerOption - getter.
function SetTriggerOption(triggerName, optionName, value) end

---Sets the current send-to location of a trigger option.
---
---If `SetTriggerOption` is called from within a plugin, the triggers for the current plugin are used, not the "global" MUSHclient timers.
---
---The supplied string will be converted to a number. It is an error if the string does not consist of digits (0 to 9).
---@param triggerName string The name of an existing trigger.
---@param optionName
---| "send_to" # Send-to location
---@param value sendto|string Option value.
---@return error_code code #
---`error_code.eInvalidObjectLabel`: The trigger name is not valid.\
---`error_code.eTriggerCannotBeEmpty`: The "match_text" cannot be empty.\
---`error_code.eScriptNameNotLocated`: The script name cannot be located in the script file.\
---`error_code.eBadRegularExpression`: The regular expression could not be evaluated.\
---`error_code.ePluginCannotSetOption`: The option is marked as non-settable (in a plugin or not).\
---`error_code.eOK`: Set OK.
---
---@see GetTriggerOption - getter.
function SetTriggerOption(triggerName, optionName, value) end

---Sets the current value of a *string* trigger option.
---
---If `SetTriggerOption` is called from within a plugin, the triggers for the current plugin are used, not the "global" MUSHclient timers.
---@param triggerName string The name of an existing trigger.
---@param optionName TriggerOptionString Option name. See below.
---@param value string Option value.
---@return error_code code #
---`error_code.eInvalidObjectLabel`: The trigger name is not valid.\
---`error_code.eTriggerCannotBeEmpty`: The "match_text" cannot be empty.\
---`error_code.eScriptNameNotLocated`: The script name cannot be located in the script file.\
---`error_code.eBadRegularExpression`: The regular expression could not be evaluated.\
---`error_code.ePluginCannotSetOption`: The option is marked as non-settable (in a plugin or not).\
---`error_code.eOK`: Set OK.
---
---@see GetTriggerOption - getter.
function SetTriggerOption(triggerName, optionName, value) end
