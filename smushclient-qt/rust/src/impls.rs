use super::ffi;
use mud_transformer::{TextStyle, UseMxp};
use smushclient::world::{AutoConnect, LogFormat, LogMode, ProxyType, ScriptRecompile};

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

macro_rules! assert_textstyle {
    ($i:ident) => {
        const _: [(); TextStyle::$i.bit() as usize] = [(); ffi::TextStyle::$i.repr as usize];
    };
}

assert_textstyle!(Blink);
assert_textstyle!(Bold);
assert_textstyle!(Highlight);
assert_textstyle!(Inverse);
assert_textstyle!(NonProportional);
assert_textstyle!(Small);
assert_textstyle!(Strikeout);
assert_textstyle!(Underline);
