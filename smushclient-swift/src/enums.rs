use mud_transformer::mxp::{self, SendTo, WorldColor};
use mud_transformer::{EffectFragment, TelnetFragment};
use smushclient::SendRequest;
use smushclient_plugins::SendTarget;

#[swift_bridge::bridge]
mod ffi {
    enum MudColor {
        Ansi(u8),
        Hex(u32),
    }

    enum SendTo {
        World,
        Input,
        Internet,
    }

    enum EffectFragment {
        Backspace,
        Beep,
        CarriageReturn,
        EraseCharacter,
        EraseLine,
    }

    enum TelnetFragment {
        Afk { challenge: String },
        Do { code: u8 },
        IacGa,
        Naws,
        Subnegotiation { code: u8, data: Vec<u8> },
        Will { code: u8 },
    }

    enum SendTarget {
        World,
        WorldDelay,
        WorldImmediate,
        Command,
        Output,
        Status,
        NotepadNew,
        NotepadAppend,
        NotepadReplace,
        Log,
        Speedwalk,
        Execute,
        Variable,
        Script,
        ScriptAfterOmit,
    }

    #[swift_bridge(swift_repr = "struct")]
    struct SendRequest {
        plugin: usize,
        #[swift_bridge(swift_name = "sendTo")]
        send_to: SendTarget,
        script: String,
        variable: String,
        text: String,
        wildcards: Vec<String>,
    }
}

pub mod bridge {
    pub use super::ffi::*;
}

impl From<WorldColor> for ffi::MudColor {
    #[inline]
    fn from(value: WorldColor) -> Self {
        match value {
            mxp::WorldColor::Ansi(code) => Self::Ansi(code),
            mxp::WorldColor::Hex(color) => Self::Hex(color.code()),
        }
    }
}

macro_rules! impl_enum_from {
    ($f:ty, $t:path, $($variant:ident),+ $(,)?) => {
        impl From<$t> for $f {
            fn from(value: $t) -> Self {
                match value {
                    $(<$t>::$variant => Self::$variant),+
                }
            }
        }
    }
}

impl_enum_from!(ffi::SendTo, SendTo, World, Input, Internet);

impl_enum_from!(
    ffi::EffectFragment,
    EffectFragment,
    Backspace,
    Beep,
    CarriageReturn,
    EraseCharacter,
    EraseLine
);

impl From<TelnetFragment> for ffi::TelnetFragment {
    fn from(value: TelnetFragment) -> Self {
        match value {
            TelnetFragment::Afk { challenge } => Self::Afk {
                challenge: String::from(&challenge),
            },
            TelnetFragment::Do { code } => Self::Do { code },
            TelnetFragment::IacGa => Self::IacGa,
            TelnetFragment::Naws => Self::Naws,
            TelnetFragment::Subnegotiation { code, data } => Self::Subnegotiation {
                code,
                data: data.to_vec(),
            },
            TelnetFragment::Will { code } => Self::Will { code },
        }
    }
}

impl_enum_from!(
    ffi::SendTarget,
    SendTarget,
    World,
    WorldDelay,
    WorldImmediate,
    Command,
    Output,
    Status,
    NotepadNew,
    NotepadAppend,
    NotepadReplace,
    Log,
    Speedwalk,
    Execute,
    Variable,
    Script,
    ScriptAfterOmit
);

impl<'a> From<SendRequest<'a>> for ffi::SendRequest {
    fn from(value: SendRequest<'a>) -> Self {
        Self {
            plugin: value.plugin,
            send_to: value.sender.send_to.into(),
            script: value.sender.script.clone(),
            variable: value.sender.variable.clone(),
            text: value.text.to_owned(),
            wildcards: value.wildcards.into_iter().map(ToOwned::to_owned).collect(),
        }
    }
}
