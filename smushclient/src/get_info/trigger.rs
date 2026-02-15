use smushclient_plugins::{PluginIndex, Trigger};

use super::visitor::InfoVisitor;
use crate::client::SmushClient;

impl SmushClient {
    pub fn trigger_info<V: InfoVisitor>(
        &self,
        index: PluginIndex,
        label: &str,
        info_type: u8,
    ) -> V::Output {
        let Some(trigger) = self
            .senders::<Trigger>(index)
            .find(|trigger| trigger.label == label)
        else {
            return V::visit_none();
        };
        match info_type {
            1 => V::visit(&trigger.pattern),
            _ => V::visit_none(),
        }
    }
}
