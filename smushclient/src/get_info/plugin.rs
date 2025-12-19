use std::path::Path;

use smushclient_plugins::PluginIndex;

use super::visitor::InfoVisitor;
use crate::client::SmushClient;

fn parse_double<V: InfoVisitor>(info: &str) -> V::Output {
    match info.parse() {
        Ok(info) => V::visit_double(info),
        Err(_) => V::visit_none(),
    }
}

impl SmushClient {
    pub fn plugin_info<V: InfoVisitor>(&self, index: PluginIndex, info_type: u8) -> V::Output {
        let Some(plugin) = self.plugins.get(index) else {
            return V::visit_none();
        };
        match info_type {
            1 => V::visit(&plugin.metadata.name),
            2 => V::visit(&plugin.metadata.author),
            3 => V::visit(&plugin.metadata.description),
            4 => V::visit(&plugin.script),
            5 => V::visit("lua"),
            6 => V::visit(&plugin.metadata.path),
            7 => V::visit(&plugin.metadata.id),
            8 => V::visit(&plugin.metadata.purpose),
            9 => V::visit(plugin.triggers.borrow().len()),
            10 => V::visit(plugin.aliases.borrow().len()),
            11 => V::visit(plugin.timers.borrow().len()),
            12 => V::visit(self.variables_len(index).unwrap_or_default()),
            13 => V::visit(plugin.metadata.written),
            14 => V::visit(plugin.metadata.modified),
            15 => V::visit(false), // save state flag
            // 16 - scripting enabled (handled by frontend)
            17 => V::visit(!plugin.disabled),
            18 => parse_double::<V>(&plugin.metadata.requires),
            19 => parse_double::<V>(&plugin.metadata.version),
            20 => V::visit(plugin.metadata.path.parent().unwrap_or(Path::new(""))),
            21 => V::visit(index),
            // 22 => V::visittime(Utc::now()), // date/time plugin installed (handled by frontend)
            // 23 - during a CallPlugin call, the ID of the calling plugin (if any) (handled by frontend)
            24 => V::visit(0.0), // Time spent on scripting in this plugin (seconds, double)
            25 => V::visit(plugin.metadata.sequence),
            _ => V::visit_none(),
        }
    }
}
