use super::ffi;
use crate::convert::Convert;
use cxx_qt_lib::QString;
use mud_transformer::mxp::{Link, SendTo};
use mud_transformer::{TextStyle, UseMxp};
use smushclient::world::{AutoConnect, LogFormat, LogMode, ProxyType, ScriptRecompile};
use smushclient_plugins::SendTarget;

impl_convert_enum!(ffi::SendTo, SendTo, Internet, World, Input);

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

impl_convert_enum!(
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

impl Default for ffi::SendTarget {
    fn default() -> Self {
        Self::World
    }
}

impl Default for ffi::Occurrence {
    fn default() -> Self {
        Self::Interval
    }
}

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

impl From<&Link> for ffi::Link {
    fn from(value: &Link) -> Self {
        Self {
            action: QString::from(&value.action),
            hint: value.hint.convert(),
            prompts: QString::from(&value.prompts.join("|")),
            sendto: value.sendto.into(),
        }
    }
}

macro_rules! assert_textstyle {
    ($i:ident) => {
        const _: [(); TextStyle::$i.bit() as usize] = [(); ffi::TextStyle::$i.repr as usize];
    };
}

assert_textstyle!(Blink);
assert_textstyle!(Bold);
assert_textstyle!(Highlight);
assert_textstyle!(NonProportional);
assert_textstyle!(Small);
assert_textstyle!(Strikeout);
assert_textstyle!(Underline);
