use super::ffi;
use cxx_qt_lib::{QList, QString, QStringList};
use mud_transformer::mxp::{Heading, Link, SendTo};
use mud_transformer::{EffectFragment, UseMxp};
use smushclient::world::{AutoConnect, LogFormat, LogMode, ProxyType, ScriptRecompile};

impl_convert_enum!(ffi::SendTo, SendTo, World, Input, Internet);

impl_convert_enum_opt!(ffi::Heading, Heading, Normal, H1, H2, H3, H4, H5, H6);

impl_convert_enum_opt!(ffi::ProxyType, ProxyType, None, Socks4, Socks5);

impl_convert_enum_opt!(ffi::AutoConnect, AutoConnect, None, Mush, Diku, Mxp);

impl_convert_enum!(
    ffi::ScriptRecompile,
    ScriptRecompile,
    Confirm,
    Always,
    Never
);

impl_convert_enum!(ffi::LogFormat, LogFormat, Text, Html, Raw);

impl_convert_enum!(ffi::LogMode, LogMode, Append, Overwrite);

impl_convert_enum!(ffi::UseMxp, UseMxp, Command, Query, Always, Never);

impl Default for ffi::ScriptRecompile {
    fn default() -> Self {
        Self::Confirm
    }
}

impl Default for ffi::LogFormat {
    fn default() -> Self {
        Self::Text
    }
}

impl Default for ffi::LogMode {
    fn default() -> Self {
        Self::Append
    }
}

impl Default for ffi::UseMxp {
    fn default() -> Self {
        Self::Command
    }
}

impl From<EffectFragment> for ffi::EffectFragment {
    fn from(value: EffectFragment) -> Self {
        match value {
            EffectFragment::Backspace => Self::Backspace,
            EffectFragment::Beep => Self::Beep,
            EffectFragment::CarriageReturn => Self::CarriageReturn,
            EffectFragment::EraseCharacter => Self::EraseCharacter,
            EffectFragment::EraseLine => Self::EraseLine,
            EffectFragment::ExpireLinks(_) => unimplemented!("<expire>"),
            EffectFragment::FileFilter(_) => unimplemented!("<filter>"),
            EffectFragment::Gauge(_) => unimplemented!("<gauge>"),
            EffectFragment::Music(_) | EffectFragment::MusicOff => unimplemented!("<music>"),
            EffectFragment::Relocate(_) => unimplemented!("<relocate>"),
            EffectFragment::Sound(_) | EffectFragment::SoundOff => unimplemented!("<sound>"),
            EffectFragment::StatusBar(_) => unimplemented!("<stat>"),
        }
    }
}

impl From<&Link> for ffi::MxpLink {
    fn from(action: &Link) -> Self {
        let hint = match &action.hint {
            Some(ref hint) => hint.as_str().into(),
            None => QString::default(),
        };
        let mut prompts = QList::default();
        for prompt in &action.prompts {
            prompts.append(QString::from(prompt));
        }
        Self {
            action: (&action.action).into(),
            hint,
            prompts: QStringList::from(&prompts),
            sendto: action.sendto.into(),
        }
    }
}
