use smushclient_plugins::{PluginIndex, Trigger};

use super::visitor::InfoVisitor;
use crate::client::SmushClient;

impl SmushClient {
    pub fn trigger_info<V: InfoVisitor>(
        &self,
        index: PluginIndex,
        label: &str,
        info_type: i64,
    ) -> V::Output {
        let Some(trigger) = self
            .senders::<Trigger>(index)
            .find(|trigger| trigger.label == label)
        else {
            return V::visit_none();
        };
        match info_type {
            1 => V::visit(&trigger.pattern),
            2 => V::visit(&trigger.text),
            3 => V::visit(&trigger.sound),
            4 => V::visit(&trigger.script),
            5 => V::visit(trigger.omit_from_log),
            6 => V::visit(trigger.omit_from_output),
            7 => V::visit(trigger.keep_evaluating),
            8 => V::visit(trigger.enabled),
            9 => V::visit(trigger.is_regex),
            10 => V::visit(trigger.ignore_case),
            11 => V::visit(trigger.repeats),
            12 => V::visit(trigger.sound_if_inactive),
            13 => V::visit(trigger.expand_variables),
            // 14 => which wildcard to send to clipboard
            15 => V::visit(trigger.send_to),
            16 => V::visit(trigger.sequence),
            17 => V::visit(0i64), // match on (colour/style)
            18 => V::visit(trigger.style_byte()),
            19 => V::visit(0), // change to colour
            // 20 => invocation count
            // 21 => times matched
            // 22 => date/time trigger last matched
            23 => V::visit(trigger.temporary),
            24 => V::visit(!self.plugins[index].metadata.is_world_plugin),
            25 => V::visit(trigger.lowercase_wildcard),
            26 => V::visit(&trigger.group),
            27 => V::visit(&trigger.variable),
            28 => V::visit(trigger.userdata),
            29 => V::visit(trigger.foreground_color),
            30 => V::visit(trigger.background_color),
            // 31 => number of matches to regular expression (most recent match)
            // 32 => last trigger text matched against
            // 33 => executing-script flag
            // 34 => script is valid (handled by frontend)
            35 => V::visit(0i64), // error number from PCRE
            36 => V::visit(trigger.one_shot),
            // 37 => time taken (in seconds) to test triggers (including un-matched ones)
            // 38 => number of attempts to match this trigger
            // 101-110 => wildcards from last match
            _ => V::visit_none(),
        }
    }
}
