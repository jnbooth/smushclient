use std::path::PathBuf;

use smushclient_plugins::LoadError;

#[derive(Debug)]
pub struct LoadFailure {
    pub error: LoadError,
    pub path: PathBuf,
}
