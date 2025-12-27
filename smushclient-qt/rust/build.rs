use std::fs::{self, File};
use std::io::Write;
use std::path::PathBuf;

use cxx_qt_build::CxxQtBuilder;

fn main() {
    let out_dir = std::env::var_os("OUT_DIR").unwrap();
    let header_dir = PathBuf::from(out_dir)
        .join("include")
        .join("smushclient-qt");

    fs::create_dir_all(&header_dir).expect("Could not create header dir");

    for (file_contents, file_name) in [
        (include_str!("../cpp/bridge/document.h"), "document.h"),
        (include_str!("../cpp/bridge/forward.h"), "forward.h"),
        (include_str!("../cpp/bridge/timekeeper.h"), "timekeeper.h"),
        (
            include_str!("../cpp/bridge/variableview.h"),
            "variableview.h",
        ),
    ] {
        let h_path = header_dir.join(file_name);
        let mut header = File::create(h_path).expect("Could not create header");
        write!(header, "{file_contents}").expect("Could not write header");
    }

    CxxQtBuilder::new()
        .cc_builder(|cc| {
            cc.include(&header_dir);
        })
        .files([
            "src/ffi/audio.rs",
            "src/ffi/client.rs",
            "src/ffi/document.rs",
            "src/ffi/plugin_details.rs",
            "src/ffi/sender.rs",
            "src/ffi/sender_map.rs",
            "src/ffi/timekeeper.rs",
            "src/ffi/util.rs",
            "src/ffi/world.rs",
            "src/ffi/variable_view.rs",
        ])
        .build();
}
