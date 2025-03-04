use std::fs::{self, File};
use std::io::Write;
use std::path::PathBuf;

use cxx_qt_build::CxxQtBuilder;

fn main() {
    let out_dir =
        std::env::var_os("CMAKE_DIR").unwrap_or_else(|| std::env::var_os("OUT_DIR").unwrap());
    let header_dir = PathBuf::from(out_dir).join("rust-gen");

    fs::create_dir_all(&header_dir).expect("Could not create header dir");

    for (file_contents, file_name) in [
        (include_str!("../cpp/bridge/document.h"), "document.h"),
        (include_str!("../cpp/bridge/forward.h"), "forward.h"),
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
        .file("src/ffi/audio.rs")
        .file("src/ffi/client.rs")
        .file("src/ffi/document.rs")
        .file("src/ffi/plugin_details.rs")
        .file("src/ffi/sender.rs")
        .file("src/ffi/sender_map.rs")
        .file("src/ffi/timekeeper.rs")
        .file("src/ffi/util.rs")
        .file("src/ffi/world.rs")
        .build();
}
