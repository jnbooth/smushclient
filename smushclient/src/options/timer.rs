use smushclient_plugins::Timer;

use super::property::BoolProperty;

#[derive(Copy, Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum TimerBool {
    // Sender
    Enabled,
    OneShot,
    Temporary,
    OmitFromOutput,
    OmitFromLog,
    // Timer
    ActiveClosed,
}

impl BoolProperty for TimerBool {
    type Target = Timer;

    fn get(self, timer: &Timer) -> bool {
        match self {
            Self::Enabled => timer.enabled,
            Self::OneShot => timer.one_shot,
            Self::Temporary => timer.temporary,
            Self::OmitFromOutput => timer.omit_from_output,
            Self::OmitFromLog => timer.omit_from_log,
            Self::ActiveClosed => timer.active_closed,
        }
    }

    fn get_mut(self, timer: &mut Timer) -> &mut bool {
        match self {
            Self::Enabled => &mut timer.enabled,
            Self::OneShot => &mut timer.one_shot,
            Self::Temporary => &mut timer.temporary,
            Self::OmitFromOutput => &mut timer.omit_from_output,
            Self::OmitFromLog => &mut timer.omit_from_log,
            Self::ActiveClosed => &mut timer.active_closed,
        }
    }
}
