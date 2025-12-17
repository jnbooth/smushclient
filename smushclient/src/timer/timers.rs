use std::collections::HashMap;

use chrono::offset::LocalResult;
use chrono::{DateTime, Days, Local, NaiveDate, NaiveTime, TimeDelta, Utc};
use smushclient_plugins::{Occurrence, PluginIndex, Timer};

use super::send_timer::{RecurringTimer, ScheduledTimer, SendTimer, TimerConstructible};
use crate::client::SmushClient;
use crate::collections::ReuseVec;
use crate::handler::TimerHandler;

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Timers<T> {
    recurring: ReuseVec<RecurringTimer<T>>,
    next_occurrences: HashMap<u16, DateTime<Utc>>,
    scheduled: Vec<ScheduledTimer<T>>,
    scheduled_times: HashMap<u16, NaiveTime>,
    cursor_date: NaiveDate,
    cursor_pos: usize,
    last_occurrences: HashMap<u16, DateTime<Utc>>,
}

impl<T> Default for Timers<T> {
    fn default() -> Self {
        Self::new()
    }
}

impl<T> Timers<T> {
    pub fn new() -> Self {
        Self {
            recurring: ReuseVec::new(),
            next_occurrences: HashMap::new(),
            scheduled: Vec::new(),
            scheduled_times: HashMap::new(),
            cursor_date: Local::now().date_naive(),
            cursor_pos: 0,
            last_occurrences: HashMap::new(),
        }
    }
}

impl<T> Timers<T> {
    pub fn clear(&mut self) {
        self.recurring.clear();
        self.next_occurrences.clear();
        self.scheduled.clear();
        self.scheduled_times.clear();
        self.cursor_date = Local::now().date_naive();
        self.cursor_pos = 0;
        self.last_occurrences.clear();
    }

    pub fn reset_plugin(&mut self, index: PluginIndex) {
        for timer in self.recurring.extract_if(|timer| timer.plugin == index) {
            self.next_occurrences.remove(&timer.id);
            self.last_occurrences.remove(&timer.id);
        }
        let scheduled_len = self.scheduled.len();
        self.scheduled.retain(|timer| {
            if timer.plugin != index {
                return true;
            }
            self.scheduled_times.remove(&timer.id);
            false
        });
        let scheduled_len_new = self.scheduled.len();
        if scheduled_len_new == scheduled_len {
            return;
        }
        self.cursor_pos = match self
            .scheduled
            .binary_search_by_key(&Local::now().time(), |timer| timer.time)
        {
            Ok(pos) | Err(pos) => pos,
        };
    }

    pub fn last_occurrence(&self, timer_id: u16) -> Option<DateTime<Utc>> {
        self.last_occurrences.get(&timer_id).copied()
    }

    pub fn next_occurrence(&self, timer_id: u16) -> Option<DateTime<Utc>> {
        if let Some(next) = self.next_occurrences.get(&timer_id) {
            return Some(*next);
        }
        let next_time = *self.scheduled_times.get(&timer_id)?;
        let now = Local::now();
        let LocalResult::Single(next_date) = now.with_time(next_time) else {
            return None;
        };
        if next_date < now
            && let Some(next_date) = next_date.checked_add_days(Days::new(1))
        {
            return Some(next_date.to_utc());
        }
        Some(next_date.to_utc())
    }
}

impl<T: TimerConstructible> Timers<T> {
    pub fn poll<H: TimerHandler<T>>(&mut self, client: &mut SmushClient, handler: &mut H) {
        if self.scheduled.is_empty() {
            return;
        }
        let now = Local::now();
        let today = now.date_naive();
        let time = now.time();
        loop {
            if self.cursor_pos >= self.scheduled.len() {
                if today > self.cursor_date {
                    self.cursor_date = today;
                    self.cursor_pos = 0;
                } else {
                    break;
                }
            }
            let send_timer = &self.scheduled[self.cursor_pos];
            if send_timer.time > time {
                break;
            }
            self.cursor_pos += 1;
            handler.send_timer(&send_timer.timer);
            if send_timer.one_shot {
                send_timer.remove(client);
                self.cursor_pos -= 1;
                self.scheduled_times.remove(&send_timer.id);
                self.scheduled.remove(self.cursor_pos);
            }
        }
    }

    pub fn start<H: TimerHandler<T>>(
        &mut self,
        index: PluginIndex,
        timer: &Timer,
        handler: &mut H,
    ) {
        if !timer.enabled {
            return;
        }
        let send_timer = SendTimer::new(index, timer);
        match timer.occurrence {
            Occurrence::Time(time) => self.insert_scheduled(ScheduledTimer::new(send_timer, time)),
            Occurrence::Interval(duration) => {
                let send_timer = RecurringTimer::new(send_timer, duration);
                let id = send_timer.id;
                let milliseconds = send_timer.milliseconds;
                let index = self.insert_recurring(send_timer);
                handler.start_timer(index, id, milliseconds);
            }
        }
    }

    pub fn finish<H: TimerHandler<T>>(
        &mut self,
        id: usize,
        client: &mut SmushClient,
        handler: &mut H,
    ) -> bool {
        let Some(send_timer) = self.recurring.get(id) else {
            return true;
        };
        self.next_occurrences.remove(&send_timer.id);
        handler.send_timer(&send_timer.timer);
        if !send_timer.one_shot {
            self.last_occurrences.insert(send_timer.id, Utc::now());
            return false;
        }
        send_timer.remove(client);
        self.recurring.remove(id);
        true
    }

    fn insert_recurring(&mut self, timer: RecurringTimer<T>) -> usize {
        let next_occurrence =
            Utc::now().checked_add_signed(TimeDelta::milliseconds(i64::from(timer.milliseconds)));
        if let Some(next_occurrence) = next_occurrence {
            self.next_occurrences.insert(timer.id, next_occurrence);
        }
        self.recurring.insert(timer)
    }

    fn insert_scheduled(&mut self, timer: ScheduledTimer<T>) {
        let time = timer.time;
        self.scheduled_times.insert(timer.id, time);
        let mut pos = match self.find_insert_index(&timer) {
            Ok(pos) => pos,
            Err(pos) => {
                self.scheduled[pos] = timer;
                return;
            }
        };
        if let Some(old_pos) = self.find_index(&timer) {
            if old_pos < pos {
                pos -= 1;
            }
            if old_pos < self.cursor_pos {
                self.cursor_pos -= 1;
            }
            self.scheduled.remove(old_pos);
        }
        self.scheduled.insert(pos, timer);

        if pos < self.cursor_pos || (pos == self.cursor_pos && time < Local::now().time()) {
            self.cursor_pos += 1;
        }
    }

    fn find_insert_index(&self, timer: &ScheduledTimer<T>) -> Result<usize, usize> {
        let id = timer.id;
        match self.scheduled.binary_search(timer) {
            Err(pos) if self.scheduled.get(pos).is_none_or(|timer| timer.id != id) => Ok(pos),
            Err(pos) | Ok(pos) => Err(pos),
        }
    }

    fn find_index(&self, timer: &ScheduledTimer<T>) -> Option<usize> {
        let id = timer.id;
        self.scheduled.iter().position(|timer| timer.id == id)
    }
}
