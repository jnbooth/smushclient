use std::pin::Pin;

use cxx_qt::CxxQtType;
use cxx_qt_lib::{QString, QVariant};
use smushclient::speedwalk;
use smushclient::world::PersistError;

use crate::ffi::{self, StringView};
use crate::get_info::InfoVisitorQVariant;

impl ffi::SmushClient {
    pub fn command_splitter(&self) -> u16 {
        let world = self.rust().client.borrow_world();
        if world.enable_command_stack {
            world.command_stack_character
        } else {
            b'\n'
        }
        .into()
    }

    pub fn evaluate_speedwalk(&self, speedwalk: StringView<'_>) -> String {
        let speedwalk = match speedwalk.to_str() {
            Ok(speedwalk) => speedwalk,
            Err(e) => return format!("*{e}"),
        };
        match self
            .rust()
            .client
            .borrow_world()
            .evaluate_speedwalk(speedwalk)
        {
            Ok(speedwalk) => speedwalk,
            Err(e) => format!("*{e}"),
        }
    }

    pub fn get_info(&self, info_type: i64) -> QVariant {
        self.rust()
            .client
            .get_info::<InfoVisitorQVariant>(info_type)
    }

    pub fn set_world(self: Pin<&mut Self>, world: &ffi::World) -> bool {
        let Ok(world) = world.rust().try_into() else {
            return false;
        };
        self.rust_mut().set_world(world)
    }

    pub fn speed_walk_prefix(&self) -> u16 {
        let world = self.rust().client.borrow_world();
        if world.enable_speed_walk {
            world.speed_walk_prefix
        } else {
            b'\0'
        }
        .into()
    }

    pub fn try_evaluate_speedwalk(&self, speedwalk: &QString) -> Result<QString, speedwalk::Error> {
        let evaluated = self
            .rust()
            .client
            .borrow_world()
            .evaluate_speedwalk(&String::from(speedwalk))?;
        Ok(QString::from(&evaluated))
    }

    pub fn try_load_world(self: Pin<&mut Self>, path: &QString) -> Result<(), PersistError> {
        self.rust_mut().load_world(String::from(path))
    }

    pub fn try_load_variables(&self, path: &QString) -> Result<bool, PersistError> {
        self.rust().load_variables(String::from(path))
    }

    pub fn try_save_world(&self, path: &QString) -> Result<(), PersistError> {
        self.rust().save_world(String::from(path))
    }

    pub fn try_save_variables(&self, path: &QString) -> Result<bool, PersistError> {
        self.rust().save_variables(String::from(path))
    }
}
