use super::ffi;
use cxx_qt_lib::{QList, QString, QStringList};
use mud_transformer::mxp::{Heading, Link, SendTo};
use mud_transformer::EffectFragment;

impl_convert_enum!(ffi::SendTo, SendTo, World, Input, Internet);

impl_convert_enum_opt!(ffi::Heading, Heading, Normal, H1, H2, H3, H4, H5, H6);

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
