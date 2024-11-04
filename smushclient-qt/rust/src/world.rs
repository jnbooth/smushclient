#![allow(clippy::result_large_err)]
use std::error::Error;
use std::pin::Pin;

use crate::adapter::TreeBuilderAdapter;
use crate::colors::Colors;
use crate::convert::Convert;
use crate::ffi;
use crate::sender::{AliasRust, TimerRust, TriggerRust};
use cxx_qt_lib::{QColor, QString};
use smushclient::world::{ColorPair, Numpad, NumpadMapping};
use smushclient::World;
use smushclient_plugins::{Alias, Occurrence, PluginLoadError, RegexError, Sender, Timer, Trigger};

#[derive(Default)]
pub struct WorldRust {
    // IP address
    pub name: QString,
    pub site: QString,
    pub port: i32,
    pub use_proxy: bool,
    pub proxy_server: QString,
    pub proxy_port: i32,
    pub proxy_username: QString,
    pub proxy_password: QString,
    pub save_world_automatically: bool,

    // Connecting
    pub player: QString,
    pub password: QString,
    pub connect_method: ffi::AutoConnect,
    pub connect_text: QString,

    // Logging
    pub log_file_preamble: QString,
    pub log_file_postamble: QString,
    pub log_format: ffi::LogFormat,
    pub log_output: bool,
    pub log_input: bool,
    pub log_notes: bool,
    pub log_mode: ffi::LogMode,
    pub auto_log_file_name: QString,
    pub log_preamble_output: QString,
    pub log_preamble_input: QString,
    pub log_preamble_notes: QString,
    pub log_postamble_output: QString,
    pub log_postamble_input: QString,
    pub log_postamble_notes: QString,

    // Timers
    pub timers: Vec<Timer>,
    pub enable_timers: bool,

    // Output
    pub show_bold: bool,
    pub show_italic: bool,
    pub show_underline: bool,
    pub indent_paras: i32,
    pub ansi_colours: Colors,
    pub display_my_input: bool,
    pub echo_text_colour: QColor,
    pub echo_background_colour: QColor,
    pub keep_commands_on_same_line: bool,
    pub new_activity_sound: QString,

    // MUD
    pub use_mxp: ffi::UseMxp,
    pub ignore_mxp_colour_changes: bool,
    pub use_custom_link_colour: bool,
    pub hyperlink_colour: QColor,
    pub mud_can_change_link_colour: bool,
    pub underline_hyperlinks: bool,
    pub hyperlink_adds_to_command_history: bool,
    pub echo_hyperlink_in_output_window: bool,
    pub terminal_identification: QString,
    pub disable_compression: bool,
    pub naws: bool,
    pub carriage_return_clears_line: bool,
    pub utf_8: bool,
    pub convert_ga_to_newline: bool,
    pub no_echo_off: bool,
    pub enable_command_stack: bool,
    pub command_stack_character: u16,

    // Triggers
    pub triggers: Vec<Trigger>,
    pub enable_triggers: bool,

    // Aliases
    pub aliases: Vec<Alias>,
    pub enable_aliases: bool,

    // Keypad
    pub numpad_0: QString,
    pub numpad_1: QString,
    pub numpad_2: QString,
    pub numpad_3: QString,
    pub numpad_4: QString,
    pub numpad_5: QString,
    pub numpad_6: QString,
    pub numpad_7: QString,
    pub numpad_8: QString,
    pub numpad_9: QString,
    pub numpad_period: QString,
    pub numpad_slash: QString,
    pub numpad_asterisk: QString,
    pub numpad_minus: QString,
    pub numpad_plus: QString,
    pub numpad_mod_0: QString,
    pub numpad_mod_1: QString,
    pub numpad_mod_2: QString,
    pub numpad_mod_3: QString,
    pub numpad_mod_4: QString,
    pub numpad_mod_5: QString,
    pub numpad_mod_6: QString,
    pub numpad_mod_7: QString,
    pub numpad_mod_8: QString,
    pub numpad_mod_9: QString,
    pub numpad_mod_period: QString,
    pub numpad_mod_slash: QString,
    pub numpad_mod_asterisk: QString,
    pub numpad_mod_minus: QString,
    pub numpad_mod_plus: QString,
    pub numpad_enable: bool,
    pub hotkey_adds_to_command_history: bool,
    pub echo_hotkey_in_output_window: bool,

    // Scripts
    pub enable_scripts: bool,
    pub world_script: QString,
    pub script_reload_option: ffi::ScriptRecompile,
    pub note_text_colour: QColor,
    pub note_background_colour: QColor,
    pub error_text_colour: QColor,
    pub error_background_colour: QColor,
}

impl_deref!(WorldRust, Colors, ansi_colours);

fn sort_refs<T: AsRef<Sender>>(items: &[T]) -> Vec<(usize, &T)> {
    let mut sorted_items: Vec<(usize, &T)> = items.iter().enumerate().collect();
    sorted_items.sort_unstable_by_key(|(_, item)| (*item).as_ref());
    sorted_items
}

impl WorldRust {
    pub fn add_alias(&mut self, alias: &AliasRust) -> Result<(), RegexError> {
        self.aliases.push(Alias::try_from(alias)?);
        Ok(())
    }

    pub fn add_timer(&mut self, timer: &TimerRust) {
        self.timers.push(Timer::from(timer));
    }

    pub fn add_trigger(&mut self, trigger: &TriggerRust) -> Result<(), RegexError> {
        self.triggers.push(Trigger::try_from(trigger)?);
        Ok(())
    }

    pub fn export_aliases(&self) -> Result<String, PluginLoadError> {
        Alias::to_xml_string(&self.aliases)
    }

    pub fn export_timers(&self) -> Result<String, PluginLoadError> {
        Timer::to_xml_string(&self.timers)
    }

    pub fn export_triggers(&self) -> Result<String, PluginLoadError> {
        Trigger::to_xml_string(&self.triggers)
    }

    pub fn import_aliases(&mut self, xml: &str) -> Result<(), PluginLoadError> {
        self.aliases.append(&mut Alias::from_xml_str(xml)?);
        Ok(())
    }

    pub fn import_timers(&mut self, xml: &str) -> Result<(), PluginLoadError> {
        self.timers.append(&mut Timer::from_xml_str(xml)?);
        Ok(())
    }

    pub fn import_triggers(&mut self, xml: &str) -> Result<(), PluginLoadError> {
        self.triggers.append(&mut Trigger::from_xml_str(xml)?);
        Ok(())
    }

    pub fn remove_alias(&mut self, index: usize) {
        if index < self.aliases.len() {
            self.aliases.remove(index);
        }
    }

    pub fn remove_timer(&mut self, index: usize) {
        if index < self.timers.len() {
            self.timers.remove(index);
        }
    }

    pub fn remove_trigger(&mut self, index: usize) {
        if index < self.triggers.len() {
            self.triggers.remove(index);
        }
    }

    pub fn replace_alias(&mut self, index: usize, alias: &AliasRust) -> Result<(), RegexError> {
        let Some(entry) = self.aliases.get_mut(index) else {
            return self.add_alias(alias);
        };
        *entry = Alias::try_from(alias)?;
        Ok(())
    }

    pub fn replace_timer(&mut self, index: usize, timer: &TimerRust) {
        let Some(entry) = self.timers.get_mut(index) else {
            self.add_timer(timer);
            return;
        };
        *entry = Timer::from(timer);
    }

    pub fn replace_trigger(
        &mut self,
        index: usize,
        trigger: &TriggerRust,
    ) -> Result<(), RegexError> {
        let Some(entry) = self.triggers.get_mut(index) else {
            return self.add_trigger(trigger);
        };
        *entry = Trigger::try_from(trigger)?;
        Ok(())
    }

    pub fn build_alias_tree(&self, builder: &mut TreeBuilderAdapter) {
        if self.aliases.is_empty() {
            return;
        }
        let sorted_items = sort_refs(&self.aliases);
        let mut last_group = "";
        for (index, item) in sorted_items {
            let group = item.group.as_str();
            if group != last_group {
                builder.start_group(&QString::from(group));
                last_group = group;
            }
            builder.start_item(index);
            builder.add_column(&QString::from(&item.label));
            builder.add_column(i64::from(item.sequence));
            builder.add_column(&QString::from(&item.pattern));
            builder.add_column(&QString::from(&item.text));
        }
    }

    pub fn build_timer_tree(&self, builder: &mut TreeBuilderAdapter) {
        if self.timers.is_empty() {
            return;
        }
        let sorted_items = sort_refs(&self.timers);
        let mut last_group = "";
        let at = QString::from("At");
        let every = QString::from("Every");
        for (index, item) in sorted_items {
            let group = item.group.as_str();
            if group != last_group {
                builder.start_group(&QString::from(group));
                last_group = group;
            }
            builder.start_item(index);
            builder.add_column(&QString::from(&item.label));
            builder.add_column(match item.occurrence {
                Occurrence::Interval(_) => &every,
                Occurrence::Time(_) => &at,
            });
            builder.add_column(&QString::from(&item.occurrence.to_string()));
            builder.add_column(&QString::from(&item.text));
        }
    }

    pub fn build_trigger_tree(&self, builder: &mut TreeBuilderAdapter) {
        if self.triggers.is_empty() {
            return;
        }
        let sorted_items = sort_refs(&self.triggers);
        let mut last_group = "";
        for (index, item) in sorted_items {
            let group = item.group.as_str();
            if group != last_group {
                builder.start_group(&QString::from(group));
                last_group = group;
            }
            builder.start_item(index);
            builder.add_column(&QString::from(&item.label));
            builder.add_column(i64::from(item.sequence));
            builder.add_column(&QString::from(&item.pattern));
            builder.add_column(&QString::from(&item.text));
        }
    }
}

impl From<&World> for WorldRust {
    fn from(world: &World) -> Self {
        let NumpadMapping {
            base: keypad,
            modified: numpad_mod,
        } = &world.numpad_shortcuts;
        Self {
            name: QString::from(&world.name),
            site: QString::from(&world.site),
            port: i32::from(world.port),
            use_proxy: world.use_proxy,
            proxy_server: QString::from(&world.proxy_server),
            proxy_port: i32::from(world.proxy_port),
            proxy_username: QString::from(&world.proxy_username),
            proxy_password: QString::from(&world.proxy_password),
            save_world_automatically: world.save_world_automatically,

            player: QString::from(&world.player),
            password: QString::from(&world.password),
            connect_method: world.connect_method.into(),
            connect_text: QString::from(&world.connect_text),

            log_file_preamble: QString::from(&world.log_file_preamble),
            log_file_postamble: QString::from(&world.log_file_postamble),
            log_format: world.log_format.into(),
            log_output: world.log_output,
            log_input: world.log_input,
            log_notes: world.log_notes,
            log_mode: world.log_mode.into(),
            auto_log_file_name: world.auto_log_file_name.convert(),
            log_preamble_output: QString::from(&world.log_preamble_output),
            log_preamble_input: QString::from(&world.log_preamble_input),
            log_preamble_notes: QString::from(&world.log_preamble_notes),
            log_postamble_output: QString::from(&world.log_postamble_output),
            log_postamble_input: QString::from(&world.log_postamble_input),
            log_postamble_notes: QString::from(&world.log_postamble_notes),

            timers: world.timers.clone(),
            enable_timers: world.enable_timers,

            show_bold: world.show_bold,
            show_italic: world.show_italic,
            show_underline: world.show_underline,
            indent_paras: i32::from(world.indent_paras),
            ansi_colours: Colors::from(&world.ansi_colours),
            display_my_input: world.display_my_input,
            echo_text_colour: world.echo_colours.foreground.convert(),
            echo_background_colour: world.echo_colours.background.convert(),
            keep_commands_on_same_line: world.keep_commands_on_same_line,
            new_activity_sound: world.new_activity_sound.convert(),

            use_mxp: world.use_mxp.into(),
            ignore_mxp_colour_changes: world.ignore_mxp_colour_changes,
            use_custom_link_colour: world.use_custom_link_colour,
            hyperlink_colour: world.hyperlink_colour.convert(),
            mud_can_change_link_colour: world.mud_can_change_link_colour,
            underline_hyperlinks: world.underline_hyperlinks,
            hyperlink_adds_to_command_history: world.hyperlink_adds_to_command_history,
            echo_hyperlink_in_output_window: world.echo_hyperlink_in_output_window,
            terminal_identification: QString::from(&world.terminal_identification),
            disable_compression: world.disable_compression,
            naws: world.naws,
            carriage_return_clears_line: world.carriage_return_clears_line,
            utf_8: world.utf_8,
            convert_ga_to_newline: world.convert_ga_to_newline,
            no_echo_off: world.no_echo_off,
            enable_command_stack: world.enable_command_stack,
            command_stack_character: world.command_stack_character,

            triggers: world.triggers.clone(),
            enable_triggers: world.enable_triggers,

            aliases: world.aliases.clone(),
            enable_aliases: world.enable_aliases,

            numpad_0: QString::from(&keypad.key_0),
            numpad_1: QString::from(&keypad.key_1),
            numpad_2: QString::from(&keypad.key_2),
            numpad_3: QString::from(&keypad.key_3),
            numpad_4: QString::from(&keypad.key_4),
            numpad_5: QString::from(&keypad.key_5),
            numpad_6: QString::from(&keypad.key_6),
            numpad_7: QString::from(&keypad.key_7),
            numpad_8: QString::from(&keypad.key_8),
            numpad_9: QString::from(&keypad.key_9),
            numpad_period: QString::from(&keypad.key_period),
            numpad_slash: QString::from(&keypad.key_slash),
            numpad_asterisk: QString::from(&keypad.key_asterisk),
            numpad_minus: QString::from(&keypad.key_minus),
            numpad_plus: QString::from(&keypad.key_plus),
            numpad_mod_0: QString::from(&numpad_mod.key_0),
            numpad_mod_1: QString::from(&numpad_mod.key_1),
            numpad_mod_2: QString::from(&numpad_mod.key_2),
            numpad_mod_3: QString::from(&numpad_mod.key_3),
            numpad_mod_4: QString::from(&numpad_mod.key_4),
            numpad_mod_5: QString::from(&numpad_mod.key_5),
            numpad_mod_6: QString::from(&numpad_mod.key_6),
            numpad_mod_7: QString::from(&numpad_mod.key_7),
            numpad_mod_8: QString::from(&numpad_mod.key_8),
            numpad_mod_9: QString::from(&numpad_mod.key_9),
            numpad_mod_period: QString::from(&numpad_mod.key_period),
            numpad_mod_slash: QString::from(&numpad_mod.key_slash),
            numpad_mod_asterisk: QString::from(&numpad_mod.key_asterisk),
            numpad_mod_minus: QString::from(&numpad_mod.key_minus),
            numpad_mod_plus: QString::from(&numpad_mod.key_plus),
            numpad_enable: world.numpad_enable,
            hotkey_adds_to_command_history: world.hotkey_adds_to_command_history,
            echo_hotkey_in_output_window: world.echo_hotkey_in_output_window,

            enable_scripts: world.enable_scripts,
            world_script: world.world_script.convert(),
            script_reload_option: world.script_reload_option.into(),
            note_text_colour: world.note_colours.foreground.convert(),
            note_background_colour: world.note_colours.background.convert(),
            error_text_colour: world.error_colours.foreground.convert(),
            error_background_colour: world.error_colours.background.convert(),
        }
    }
}

impl TryFrom<&WorldRust> for World {
    type Error = crate::convert::OutOfRangeError;

    fn try_from(value: &WorldRust) -> Result<Self, Self::Error> {
        let mut world = Self {
            name: String::from(&value.name),
            site: String::from(&value.site),
            port: u16::try_from(value.port)?,
            use_proxy: value.use_proxy,
            proxy_server: String::from(&value.proxy_server),
            proxy_port: u16::try_from(value.proxy_port)?,
            proxy_username: String::from(&value.proxy_username),
            proxy_password: String::from(&value.proxy_password),
            save_world_automatically: value.save_world_automatically,

            player: String::from(&value.player),
            password: String::from(&value.password),
            connect_method: value.connect_method.try_into()?,
            connect_text: String::from(&value.connect_text),

            log_file_preamble: String::from(&value.log_file_preamble),
            log_file_postamble: String::from(&value.log_file_postamble),
            log_format: value.log_format.try_into()?,
            log_output: value.log_output,
            log_input: value.log_input,
            log_notes: value.log_notes,
            log_mode: value.log_mode.try_into()?,
            auto_log_file_name: value.auto_log_file_name.convert(),
            log_preamble_output: String::from(&value.log_preamble_output),
            log_preamble_input: String::from(&value.log_preamble_input),
            log_preamble_notes: String::from(&value.log_preamble_notes),
            log_postamble_output: String::from(&value.log_postamble_output),
            log_postamble_input: String::from(&value.log_postamble_input),
            log_postamble_notes: String::from(&value.log_postamble_notes),

            timers: value.timers.clone(),
            enable_timers: value.enable_timers,

            show_bold: value.show_bold,
            show_italic: value.show_italic,
            show_underline: value.show_underline,
            indent_paras: u8::try_from(value.indent_paras)?,
            ansi_colours: (&value.ansi_colours).into(),
            display_my_input: value.display_my_input,
            echo_colours: ColorPair {
                foreground: value.echo_text_colour.convert(),
                background: value.echo_background_colour.convert(),
            },
            keep_commands_on_same_line: value.keep_commands_on_same_line,
            new_activity_sound: value.new_activity_sound.convert(),

            use_mxp: value.use_mxp.try_into()?,
            ignore_mxp_colour_changes: value.ignore_mxp_colour_changes,
            use_custom_link_colour: value.use_custom_link_colour,
            hyperlink_colour: value.hyperlink_colour.convert(),
            mud_can_change_link_colour: value.mud_can_change_link_colour,
            underline_hyperlinks: value.underline_hyperlinks,
            hyperlink_adds_to_command_history: value.hyperlink_adds_to_command_history,
            echo_hyperlink_in_output_window: value.echo_hyperlink_in_output_window,
            terminal_identification: String::from(&value.terminal_identification),
            disable_compression: value.disable_compression,
            naws: value.naws,
            carriage_return_clears_line: value.carriage_return_clears_line,
            utf_8: value.utf_8,
            convert_ga_to_newline: value.convert_ga_to_newline,
            no_echo_off: value.no_echo_off,
            enable_command_stack: value.enable_command_stack,
            command_stack_character: value.command_stack_character,

            triggers: value.triggers.clone(),
            enable_triggers: value.enable_triggers,

            aliases: value.aliases.clone(),
            enable_aliases: value.enable_aliases,

            numpad_shortcuts: NumpadMapping {
                base: Numpad {
                    key_0: String::from(&value.numpad_0),
                    key_1: String::from(&value.numpad_1),
                    key_2: String::from(&value.numpad_2),
                    key_3: String::from(&value.numpad_3),
                    key_4: String::from(&value.numpad_4),
                    key_5: String::from(&value.numpad_5),
                    key_6: String::from(&value.numpad_6),
                    key_7: String::from(&value.numpad_7),
                    key_8: String::from(&value.numpad_8),
                    key_9: String::from(&value.numpad_9),
                    key_period: String::from(&value.numpad_period),
                    key_slash: String::from(&value.numpad_slash),
                    key_asterisk: String::from(&value.numpad_asterisk),
                    key_minus: String::from(&value.numpad_minus),
                    key_plus: String::from(&value.numpad_plus),
                },
                modified: Numpad {
                    key_0: String::from(&value.numpad_mod_0),
                    key_1: String::from(&value.numpad_mod_1),
                    key_2: String::from(&value.numpad_mod_2),
                    key_3: String::from(&value.numpad_mod_3),
                    key_4: String::from(&value.numpad_mod_4),
                    key_5: String::from(&value.numpad_mod_5),
                    key_6: String::from(&value.numpad_mod_6),
                    key_7: String::from(&value.numpad_mod_7),
                    key_8: String::from(&value.numpad_mod_8),
                    key_9: String::from(&value.numpad_mod_9),
                    key_period: String::from(&value.numpad_mod_period),
                    key_slash: String::from(&value.numpad_mod_slash),
                    key_asterisk: String::from(&value.numpad_mod_asterisk),
                    key_minus: String::from(&value.numpad_mod_minus),
                    key_plus: String::from(&value.numpad_mod_plus),
                },
            },
            numpad_enable: value.numpad_enable,
            hotkey_adds_to_command_history: value.hotkey_adds_to_command_history,
            echo_hotkey_in_output_window: value.echo_hotkey_in_output_window,

            enable_scripts: value.enable_scripts,
            world_script: value.world_script.convert(),
            script_reload_option: value.script_reload_option.try_into()?,
            note_colours: ColorPair {
                foreground: value.note_text_colour.convert(),
                background: value.note_background_colour.convert(),
            },
            error_colours: ColorPair {
                foreground: value.error_text_colour.convert(),
                background: value.error_background_colour.convert(),
            },

            plugins: Vec::new(),
        };
        world.aliases.sort_unstable();
        world.timers.sort_unstable();
        world.triggers.sort_unstable();
        Ok(world)
    }
}

trait IntoResultString {
    fn result(self) -> QString;
}

impl<E: Error> IntoResultString for Result<(), E> {
    fn result(self) -> QString {
        match self {
            Ok(()) => QString::default(),
            Err(e) => QString::from(&e.to_string()),
        }
    }
}

impl<E: Error> IntoResultString for Result<String, E> {
    fn result(self) -> QString {
        match self {
            Ok(s) => QString::from(&s),
            Err(e) => QString::from(&e.to_string()),
        }
    }
}

impl ffi::World {
    pub fn add_alias(self: Pin<&mut Self>, alias: &ffi::Alias) -> QString {
        self.cxx_qt_ffi_rust_mut()
            .add_alias(alias.cxx_qt_ffi_rust())
            .result()
    }

    pub fn add_timer(self: Pin<&mut Self>, timer: &ffi::Timer) -> QString {
        self.cxx_qt_ffi_rust_mut()
            .add_timer(timer.cxx_qt_ffi_rust());
        QString::default()
    }

    pub fn add_trigger(self: Pin<&mut Self>, trigger: &ffi::Trigger) -> QString {
        self.cxx_qt_ffi_rust_mut()
            .add_trigger(trigger.cxx_qt_ffi_rust())
            .result()
    }

    pub fn export_aliases(&self) -> QString {
        self.cxx_qt_ffi_rust().export_aliases().result()
    }

    pub fn export_timers(&self) -> QString {
        self.cxx_qt_ffi_rust().export_timers().result()
    }

    pub fn export_triggers(&self) -> QString {
        self.cxx_qt_ffi_rust().export_triggers().result()
    }

    pub fn import_aliases(self: Pin<&mut Self>, xml: &QString) -> QString {
        self.cxx_qt_ffi_rust_mut()
            .import_aliases(&String::from(xml))
            .result()
    }

    pub fn import_timers(self: Pin<&mut Self>, xml: &QString) -> QString {
        self.cxx_qt_ffi_rust_mut()
            .import_timers(&String::from(xml))
            .result()
    }

    pub fn import_triggers(self: Pin<&mut Self>, xml: &QString) -> QString {
        self.cxx_qt_ffi_rust_mut()
            .import_triggers(&String::from(xml))
            .result()
    }

    pub fn num_aliases(&self) -> usize {
        self.cxx_qt_ffi_rust().aliases.len()
    }

    pub fn num_timers(&self) -> usize {
        self.cxx_qt_ffi_rust().timers.len()
    }

    pub fn num_triggers(&self) -> usize {
        self.cxx_qt_ffi_rust().triggers.len()
    }

    pub fn remove_alias(self: Pin<&mut Self>, index: usize) {
        self.cxx_qt_ffi_rust_mut().remove_alias(index);
    }

    pub fn remove_timer(self: Pin<&mut Self>, index: usize) {
        self.cxx_qt_ffi_rust_mut().remove_timer(index);
    }

    pub fn remove_trigger(self: Pin<&mut Self>, index: usize) {
        self.cxx_qt_ffi_rust_mut().remove_trigger(index);
    }

    pub fn replace_alias(self: Pin<&mut Self>, index: usize, alias: &ffi::Alias) -> QString {
        self.cxx_qt_ffi_rust_mut()
            .replace_alias(index, alias.cxx_qt_ffi_rust())
            .result()
    }

    pub fn replace_timer(self: Pin<&mut Self>, index: usize, timer: &ffi::Timer) -> QString {
        self.cxx_qt_ffi_rust_mut()
            .replace_timer(index, timer.cxx_qt_ffi_rust());
        QString::default()
    }

    pub fn replace_trigger(self: Pin<&mut Self>, index: usize, trigger: &ffi::Trigger) -> QString {
        self.cxx_qt_ffi_rust_mut()
            .replace_trigger(index, trigger.cxx_qt_ffi_rust())
            .result()
    }

    pub fn build_alias_tree(&self, builder: Pin<&mut ffi::TreeBuilder>) {
        self.cxx_qt_ffi_rust().build_alias_tree(&mut builder.into());
    }

    pub fn build_timer_tree(&self, builder: Pin<&mut ffi::TreeBuilder>) {
        self.cxx_qt_ffi_rust().build_timer_tree(&mut builder.into());
    }

    pub fn build_trigger_tree(&self, builder: Pin<&mut ffi::TreeBuilder>) {
        self.cxx_qt_ffi_rust()
            .build_trigger_tree(&mut builder.into());
    }
}
