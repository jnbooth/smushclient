use super::visitor::InfoVisitor;
use crate::client::SmushClient;
use crate::timer::Timers;
use chrono::Utc;
use smushclient_plugins::{Occurrence, PluginIndex, SendTarget, Timer};

impl SmushClient {
    pub fn timer_info<V: InfoVisitor, T>(
        &self,
        index: PluginIndex,
        label: &str,
        info_type: u8,
        timers: &Timers<T>,
    ) -> V::Output {
        let Some(timer) = self
            .senders::<Timer>(index)
            .find(|timer| timer.label == label)
        else {
            return V::visit_none();
        };
        match info_type {
            1 => V::visit(timer.occurrence.hour()),
            2 => V::visit(timer.occurrence.minute()),
            3 => V::visit(timer.occurrence.second()),
            4 => V::visit(&timer.text),
            5 => V::visit(&timer.script),
            6 => V::visit(timer.enabled),
            7 => V::visit(timer.one_shot),
            8 => V::visit(matches!(timer.occurrence, Occurrence::Time(_))),
            // 9 => invocation count
            // 10 => times matched
            11 => V::visit(timers.last_occurrence(timer.id)),
            12 => V::visit(timers.next_occurrence(timer.id)),
            13 => V::visit(
                timers
                    .next_occurrence(timer.id)
                    .map(|occurrence| occurrence.signed_duration_since(Utc::now()).num_seconds()),
            ),
            14 => V::visit(timer.temporary),
            15 => V::visit(timer.send_to == SendTarget::Speedwalk),
            16 => V::visit(timer.send_to == SendTarget::Output),
            17 => V::visit(timer.active_closed),
            18 => V::visit(false), // timer was included from an include file
            19 => V::visit(&timer.group),
            20 => V::visit(timer.send_to),
            21 => V::visit(timer.userdata),
            22 => V::visit(&timer.label),
            23 => V::visit(timer.omit_from_output),
            24 => V::visit(timer.omit_from_log),
            // 25 => is executing
            // 26 => script is valid (handled by client)
            _ => V::visit_none(),
        }
    }
}
