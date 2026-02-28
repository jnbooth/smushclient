use smushclient_plugins::{Alias, PluginIndex};

use super::visitor::InfoVisitor;
use crate::client::SmushClient;

impl SmushClient {
    pub fn alias_info<V: InfoVisitor>(
        &self,
        index: PluginIndex,
        label: &str,
        info_type: i64,
    ) -> V::Output {
        let Some(alias) = self
            .senders::<Alias>(index)
            .find(|alias| alias.label == label)
        else {
            return V::visit_none();
        };
        match info_type {
            1 => V::visit(&alias.pattern),
            2 => V::visit(&alias.text),
            3 => V::visit(&alias.script),
            4 => V::visit(alias.omit_from_log),
            5 => V::visit(alias.omit_from_output),
            6 => V::visit(alias.enabled),
            7 => V::visit(alias.is_regex),
            8 => V::visit(alias.ignore_case),
            9 => V::visit(alias.expand_variables),
            // 10 => invocation count
            // 11 => times matched
            12 => V::visit(alias.menu),
            // 13 => date/time alias last matched
            14 => V::visit(alias.temporary),
            15 => V::visit(!self.plugins[index].metadata.is_world_plugin),
            16 => V::visit(&alias.group),
            17 => V::visit(&alias.variable),
            18 => V::visit(alias.send_to),
            19 => V::visit(alias.keep_evaluating),
            20 => V::visit(alias.sequence),
            21 => V::visit(alias.echo_alias),
            22 => V::visit(alias.omit_from_command_history),
            23 => V::visit(alias.userdata),
            // 24 => number of matches to regular expression (most recent match)
            // 25 => last alias text matched against
            // 26 => executing-script flag
            // 27 => script is valid (handled by frontend)
            28 => V::visit(0i64), // error number from PCRE when evaluating last match
            29 => V::visit(alias.one_shot),
            // 30 => time taken (in seconds) to test aliases (including un-matched ones)
            // 31 => number of attempts to match this alias (long)
            // 101-110 => wildcards from last match
            _ => V::visit_none(),
        }
    }
}
