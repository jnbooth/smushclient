use smushclient_plugins::{Alias, PluginIndex};

use super::visitor::InfoVisitor;
use crate::client::SmushClient;

impl SmushClient {
    pub fn alias_info<V: InfoVisitor>(
        &self,
        index: PluginIndex,
        label: &str,
        info_type: u8,
    ) -> V::Output {
        let Some(alias) = self
            .senders::<Alias>(index)
            .find(|alias| alias.label == label)
        else {
            return V::visit_none();
        };
        match info_type {
            1 => V::visit(&alias.pattern),
            _ => V::visit_none(),
        }
    }
}
