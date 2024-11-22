use std::fs::{self, File};
use std::io::Write;
use std::path::PathBuf;

use cxx_qt_build::{CxxQtBuilder, QmlModule};

fn main() {
    let out_dir =
        std::env::var_os("CMAKE_DIR").unwrap_or_else(|| std::env::var_os("OUT_DIR").unwrap());
    let header_dir = PathBuf::from(out_dir).join("rust-gen");

    fs::create_dir_all(&header_dir).expect("Could not create header dir");

    for (file_contents, file_name) in [
        (include_str!("../cpp/bridge/document.h"), "document.h"),
        (include_str!("../cpp/bridge/timekeeper.h"), "timekeeper.h"),
    ] {
        let h_path = header_dir.join(file_name);
        let mut header = File::create(h_path).expect("Could not create header");
        write!(header, "{file_contents}").expect("Could not write header");
    }

    CxxQtBuilder::new()
        .cc_builder(|cc| {
            cc.include(&header_dir);
        })
        .qml_module::<&str, &str>(QmlModule {
            uri: "com.jnbooth.smushclient",
            rust_files: &["src/bridge.rs"],
            qml_files: &[],
            ..Default::default()
        })
        .build();
}
