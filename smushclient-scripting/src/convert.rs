use std::borrow::Cow;
use std::collections::{BTreeMap, HashMap};
use std::ffi::{OsStr, OsString};
use std::hash::{BuildHasher, Hash};
use std::path::{Path, PathBuf};
use std::str;
use std::time::{Duration, SystemTime};

use mlua::{self, FromLuaMulti, IntoLua, IntoLuaMulti, LightUserData, Lua, MultiValue, Value};

#[inline]
fn create_string<'lua>(lua: &'lua Lua, value: &[u8]) -> mlua::Result<Value<'lua>> {
    lua.create_string(value).map(Value::String)
}

pub trait ScriptRes: for<'lua> FromLuaMulti<'lua> {}
impl<T: for<'lua> FromLuaMulti<'lua>> ScriptRes for T {}

pub trait ScriptArg {
    fn to_arg(self, lua: &Lua) -> mlua::Result<Value>;
}

impl<T: ScriptArg> ScriptArg for Option<T> {
    fn to_arg(self, lua: &Lua) -> mlua::Result<Value> {
        match self {
            Some(val) => val.to_arg(lua),
            None => Ok(Value::Nil),
        }
    }
}

impl ScriptArg for &[u8] {
    fn to_arg(self, lua: &Lua) -> mlua::Result<Value> {
        create_string(lua, self)
    }
}

impl ScriptArg for &String {
    fn to_arg(self, lua: &Lua) -> mlua::Result<Value> {
        create_string(lua, self.as_bytes())
    }
}

impl<'a> ScriptArg for &Cow<'a, str> {
    fn to_arg(self, lua: &Lua) -> mlua::Result<Value> {
        create_string(lua, self.as_bytes())
    }
}

impl<'a> ScriptArg for Cow<'a, str> {
    fn to_arg(self, lua: &Lua) -> mlua::Result<Value> {
        create_string(lua, self.as_bytes())
    }
}

impl ScriptArg for &OsStr {
    fn to_arg(self, lua: &Lua) -> mlua::Result<Value> {
        create_string(lua, self.as_encoded_bytes())
    }
}

impl ScriptArg for &OsString {
    fn to_arg(self, lua: &Lua) -> mlua::Result<Value> {
        create_string(lua, self.as_encoded_bytes())
    }
}

impl ScriptArg for &Path {
    fn to_arg(self, lua: &Lua) -> mlua::Result<Value> {
        create_string(lua, self.as_os_str().as_encoded_bytes())
    }
}

impl ScriptArg for &PathBuf {
    fn to_arg(self, lua: &Lua) -> mlua::Result<Value> {
        create_string(lua, self.as_os_str().as_encoded_bytes())
    }
}

impl ScriptArg for &Option<PathBuf> {
    fn to_arg(self, lua: &Lua) -> mlua::Result<Value> {
        match self {
            Some(path) => create_string(lua, path.as_os_str().as_encoded_bytes()),
            None => create_string(lua, b""),
        }
    }
}

impl ScriptArg for &Duration {
    fn to_arg(self, lua: &Lua) -> mlua::Result<Value> {
        self.as_millis().to_arg(lua)
    }
}

impl ScriptArg for &SystemTime {
    fn to_arg(self, lua: &Lua) -> mlua::Result<Value> {
        self.duration_since(SystemTime::UNIX_EPOCH)
            .expect("invalid system time")
            .to_arg(lua)
    }
}

macro_rules! impl_arg {
    ($t:ty) => {
        impl ScriptArg for $t {
            #[inline]
            fn to_arg(self, lua: &Lua) -> mlua::Result<Value> {
                IntoLua::into_lua(self, lua)
            }
        }
    };
}

impl_arg!(bool);
impl_arg!(String);
impl_arg!(&'_ str);
impl_arg!(std::ffi::CString);
impl_arg!(&'_ std::ffi::CStr);
impl_arg!(i8);
impl_arg!(u8);
impl_arg!(i16);
impl_arg!(u16);
impl_arg!(i32);
impl_arg!(u32);
impl_arg!(i64);
impl_arg!(u64);
impl_arg!(i128);
impl_arg!(u128);
impl_arg!(isize);
impl_arg!(usize);
impl_arg!(f32);
impl_arg!(f64);
impl_arg!(LightUserData);

macro_rules! impl_arg_deref {
    ($t:ty) => {
        impl<T> ScriptArg for $t
        where
            for<'a> &'a T: ScriptArg,
        {
            fn to_arg(self, lua: &Lua) -> mlua::Result<Value> {
                (*self).to_arg(lua)
            }
        }
        impl<T> ScriptArg for &$t
        where
            for<'a> &'a T: ScriptArg,
        {
            fn to_arg(self, lua: &Lua) -> mlua::Result<Value> {
                (**self).to_arg(lua)
            }
        }
    };
}

impl_arg_deref!(std::rc::Rc<T>);

fn create_table<K, V, I>(lua: &Lua, cont: I) -> mlua::Result<Value>
where
    K: ScriptArg,
    V: ScriptArg,
    I: IntoIterator<Item = (K, V)>,
{
    let args: mlua::Result<Vec<_>> = cont
        .into_iter()
        .map(|(k, v)| Ok((k.to_arg(lua)?, v.to_arg(lua)?)))
        .collect();
    lua.create_table_from(args?).map(Value::Table)
}

fn create_sequence<T, I>(lua: &Lua, cont: I) -> mlua::Result<Value>
where
    T: ScriptArg,
    I: IntoIterator<Item = T>,
{
    let args: mlua::Result<Vec<_>> = cont.into_iter().map(|x| x.to_arg(lua)).collect();
    lua.create_sequence_from(args).map(Value::Table)
}

impl<T: ScriptArg> ScriptArg for Vec<T> {
    fn to_arg(self, lua: &Lua) -> mlua::Result<Value> {
        create_sequence(lua, self)
    }
}

impl ScriptArg for [(); 0] {
    fn to_arg(self, lua: &Lua) -> mlua::Result<Value> {
        lua.create_table().map(Value::Table)
    }
}

impl<K: ScriptArg + Eq + Hash, V: ScriptArg, S: BuildHasher> ScriptArg for HashMap<K, V, S> {
    fn to_arg(self, lua: &Lua) -> mlua::Result<Value> {
        create_table(lua, self)
    }
}

impl<K: ScriptArg + Ord, V: ScriptArg> ScriptArg for BTreeMap<K, V> {
    fn to_arg(self, lua: &Lua) -> mlua::Result<Value> {
        create_table(lua, self)
    }
}

pub trait ScriptArgs {
    fn to_args(self, lua: &Lua) -> mlua::Result<MultiValue>;
}

impl ScriptArgs for () {
    fn to_args(self, _: &Lua) -> mlua::Result<MultiValue> {
        Ok(MultiValue::new())
    }
}

impl<T: ScriptArg> ScriptArgs for T {
    fn to_args(self, lua: &Lua) -> mlua::Result<MultiValue> {
        IntoLuaMulti::into_lua_multi(self.to_arg(lua), lua)
    }
}

macro_rules! peel_args {
    ($name:ident, $($other:ident,)*) => (impl_args! { $($other,)* })
}

macro_rules! impl_args {
    () => ();
    ($($name:ident,)+) => {
        impl<$($name,)*> ScriptArgs for ($($name,)*)
        where $($name: ScriptArg,)*
        {
            #[allow(non_snake_case)]
            fn to_args(self, lua: &Lua) -> mlua::Result<MultiValue> {
                let ($($name,)*) = self;

                IntoLuaMulti::into_lua_multi((
                    $(ScriptArg::to_arg($name, lua)?,)*
                ), lua)
            }
        }
        peel_args! { $($name,)+ }
    }
}

impl_args! {A, B, C, D, E, F, G, H, I, J, K, L, M, N, O,}