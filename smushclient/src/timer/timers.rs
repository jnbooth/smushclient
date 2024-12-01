use super::send_timer::{RecurringTimer, ScheduledTimer, SendTimer, TimerConstructible};
use crate::client::SmushClient;
use crate::collections::ReuseVec;
use crate::handler::TimerHandler;
use chrono::{Local, NaiveDate};
use smushclient_plugins::{Occurrence, PluginIndex, Timer};

#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct Timers<T> {
    recurring: ReuseVec<RecurringTimer<T>>,
    scheduled: Vec<ScheduledTimer<T>>,
    cursor_date: NaiveDate,
    cursor_pos: usize,
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
            scheduled: Vec::new(),
            cursor_date: Local::now().date_naive(),
            cursor_pos: 0,
        }
    }
}

impl<T: TimerConstructible> Timers<T> {
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
            Occurrence::Time(time) => {
                let id = send_timer.id;
                let send_timer = ScheduledTimer::new(send_timer, time);
                let mut pos = match self.scheduled.binary_search(&send_timer) {
                    Err(pos) if !self.scheduled.get(pos).is_some_and(|timer| timer.id == id) => pos,
                    Err(pos) | Ok(pos) => {
                        self.scheduled[pos] = send_timer;
                        return;
                    }
                };
                if let Some(old_pos) = self
                    .scheduled
                    .iter()
                    .position(|send_timer| send_timer.id == id)
                {
                    if old_pos < pos {
                        pos -= 1;
                    }
                    if old_pos < self.cursor_pos {
                        self.cursor_pos -= 1;
                    }
                    self.scheduled.remove(old_pos);
                }
                self.scheduled.insert(pos, send_timer);

                if pos < self.cursor_pos || (pos == self.cursor_pos && time < Local::now().time()) {
                    self.cursor_pos += 1;
                }
            }
            Occurrence::Interval(duration) => {
                let id = send_timer.id;
                let send_timer = RecurringTimer::new(send_timer, duration);
                let milliseconds = send_timer.milliseconds;
                let index = self.recurring.insert(send_timer);
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
        handler.send_timer(&send_timer.timer);
        if !send_timer.one_shot {
            return false;
        }
        send_timer.remove(client);
        self.recurring.remove(id);
        true
    }

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
            self.cursor_pos += 1;
            if send_timer.time > time {
                break;
            }
            handler.send_timer(&send_timer.timer);
            if send_timer.one_shot {
                send_timer.remove(client);
                self.cursor_pos -= 1;
                self.scheduled.remove(self.cursor_pos);
            }
        }
    }
}
