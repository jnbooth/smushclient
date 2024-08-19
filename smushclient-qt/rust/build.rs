use std::str;

use cxx_qt_build::{CxxQtBuilder, QmlModule};

fn main() {
    CxxQtBuilder::new()
        .qml_module::<&str, &str>(QmlModule {
            uri: "com.jnbooth.smushclient",
            rust_files: &["src/lib.rs"],
            qml_files: &[],
            ..Default::default()
        })
        .build();
}
