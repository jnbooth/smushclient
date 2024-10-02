use std::path::Path;

use chrono::Utc;
use smushclient_plugins::{Alias, PluginIndex, Timer, Trigger};

use super::visitor::InfoVisitor;
use crate::SmushClient;

fn parse_double<V: InfoVisitor>(info: &str) -> V::Output {
    match info.parse() {
        Ok(info) => V::visit_double(info),
        Err(_) => V::visit_none(),
    }
}

impl SmushClient {
    pub fn plugin_info<V: InfoVisitor>(&self, index: PluginIndex, info_type: u8) -> V::Output {
        let Some(plugin) = self.plugins.plugin(index) else {
            return V::visit_none();
        };
        match info_type {
            1 => V::visit_str(&plugin.metadata.name),
            2 => V::visit_str(&plugin.metadata.author),
            3 => V::visit_str(&plugin.metadata.description),
            4 => V::visit_str(&plugin.script),
            5 => V::visit_str("lua"),
            6 => V::visit_path(&plugin.metadata.path),
            7 => V::visit_str(&plugin.metadata.id),
            8 => V::visit_str(&plugin.metadata.purpose),
            9 => V::visit_usize(self.plugins.indexer::<Trigger>().count(index)),
            10 => V::visit_usize(self.plugins.indexer::<Alias>().count(index)),
            11 => V::visit_usize(self.plugins.indexer::<Timer>().count(index)),
            12 => V::visit_usize(self.variables_len(index).unwrap_or_default()),
            13 => V::visit_date(plugin.metadata.written),
            14 => V::visit_date(plugin.metadata.modified),
            15 => V::visit_bool(false), // save state flag
            // 16 - scripting enabled (handled by frontend)
            17 => V::visit_bool(!plugin.disabled),
            18 => parse_double::<V>(&plugin.metadata.requires),
            19 => parse_double::<V>(&plugin.metadata.version),
            20 => V::visit_path(plugin.metadata.path.parent().unwrap_or(Path::new(""))),
            21 => V::visit_usize(index),
            22 => V::visit_datetime(Utc::now()), // date/time plugin installed (date)
            // 23 - during a CallPlugin call, the ID of the calling plugin (if any) (handled by frontend)
            24 => V::visit_double(0.0), // Time spent on scripting in this plugin (seconds, double)
            25 => V::visit_i16(plugin.metadata.sequence),
            _ => V::visit_none(),
        }
    }
}
