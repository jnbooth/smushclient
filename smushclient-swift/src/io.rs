use smushclient::World;

use crate::error::StringifyResultError;

pub fn create_world() -> World<'static> {
    World::new()
}

pub fn read_world(data: &[u8]) -> Result<World<'static>, String> {
    World::load(data).str()
}

#[allow(clippy::needless_pass_by_value)]
pub fn write_world(world: World<'static>) -> Result<Vec<u8>, String> {
    let mut vec = Vec::new();
    world.save(&mut vec).str()?;
    Ok(vec)
}
