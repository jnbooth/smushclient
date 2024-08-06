use crate::convert::Convert;
use mud_transformer::mxp::SendTo;
use mud_transformer::{EffectFragment, TelnetFragment};

#[swift_bridge::bridge]
pub mod ffi {
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
}

impl_convert_enum!(ffi::SendTo, SendTo, World, Input, Internet);

impl_convert_enum!(
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

impl Clone for ffi::TelnetFragment {
    fn clone(&self) -> Self {
        match self {
            Self::Afk { challenge } => Self::Afk {
                challenge: challenge.clone(),
            },
            Self::Do { code } => Self::Do { code: *code },
            Self::IacGa => Self::IacGa,
            Self::Naws => Self::Naws,
            Self::Subnegotiation { code, data } => Self::Subnegotiation {
                code: *code,
                data: data.clone(),
            },
            Self::Will { code } => Self::Will { code: *code },
        }
    }
}
