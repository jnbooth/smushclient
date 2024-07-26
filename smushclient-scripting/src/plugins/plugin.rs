use std::cmp::Ordering;
use std::{mem, str};

use enumeration::{Enum, EnumSet};
use mlua::{self, AnyUserData, FromLuaMulti, Function, Lua, Value};

use super::{PluginMetadata, PluginPack};
use crate::callback::Callback;
use crate::convert::ScriptArgs;
use crate::send::{Alias, Timer, Trigger};

pub struct Plugin {
    pub metadata: PluginMetadata,
    /// Which callbacks it responds to.
    pub callbacks: EnumSet<Callback>,
    pub triggers: Vec<Trigger>,
    pub aliases: Vec<Alias>,
    pub timers: Vec<Timer>,
}

impl PartialEq for Plugin {
    fn eq(&self, other: &Self) -> bool {
        self.metadata.eq(&other.metadata)
    }
}

impl Eq for Plugin {}

impl PartialOrd for Plugin {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

impl Ord for Plugin {
    fn cmp(&self, other: &Self) -> Ordering {
        self.metadata.cmp(&other.metadata)
    }
}

impl Plugin {
    pub fn load(engine: &Lua, pack: PluginPack) -> mlua::Result<Self> {
        engine.load(&pack.script).exec()?;
        let globals = engine.globals();

        let callbacks = Callback::enumerate(..)
            .filter(|cb| {
                matches!(
                    globals.raw_get(format!("{:?}", cb)),
                    Ok(Value::Function(..))
                )
            })
            .collect();

        mem::drop(globals);

        Ok(Plugin {
            metadata: pack.metadata,
            callbacks,
            triggers: pack.triggers,
            aliases: pack.aliases,
            timers: pack.timers,
        })
    }

    pub fn with_userdata<T, R, F>(&self, engine: &Lua, key: &str, f: F) -> mlua::Result<R>
    where
        T: 'static,
        F: FnOnce(&T) -> R,
    {
        let global_ref = engine.globals().raw_get::<_, AnyUserData>(key)?;
        let global_val = global_ref.borrow()?;
        Ok(f(&global_val))
    }

    pub fn with_userdata_mut<T, R, F>(&self, engine: &Lua, key: &str, f: F) -> mlua::Result<R>
    where
        T: 'static,
        F: FnOnce(&mut T) -> R,
    {
        let global_ref = engine.globals().raw_get::<_, AnyUserData>(key)?;
        let mut global_val = global_ref.borrow_mut()?;
        Ok(f(&mut global_val))
    }

    pub fn invoke<'lua, A, R>(
        &'lua self,
        engine: &'lua Lua,
        fn_name: &str,
        args: A,
    ) -> mlua::Result<R>
    where
        A: ScriptArgs,
        R: FromLuaMulti<'lua>,
    {
        let f: Function<'lua> = engine.globals().raw_get(fn_name)?;
        f.call(args.to_args(engine)?)
    }

    pub fn call<'lua, A, R>(&'lua self, engine: &'lua Lua, cb: Callback, args: A) -> mlua::Result<R>
    where
        A: ScriptArgs,
        R: FromLuaMulti<'lua>,
    {
        self.invoke(engine, &format!("{:?}", cb), args)
    }
}
