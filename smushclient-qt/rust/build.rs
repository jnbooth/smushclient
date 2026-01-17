use std::ffi::OsStr;
use std::fs::{self, File};
use std::io::Write;
use std::path::{Path, PathBuf};

use cxx_qt_build::CxxQtBuilder;

macro_rules! include_header {
    ($file:literal) => {
        (include_bytes!($file), Path::new($file).file_name().unwrap())
    };
}

fn main() {
    let out_dir = std::env::var_os("OUT_DIR").unwrap();
    let header_dir = PathBuf::from(out_dir)
        .join("include")
        .join("smushclient-qt");

    fs::create_dir_all(&header_dir).expect("Could not create header dir");

    let header_files: &[(&[u8], &OsStr)] = &[
        include_header!("../cpp/bridge/document.h"),
        include_header!("../cpp/bridge/forward.h"),
        include_header!("../cpp/bridge/timekeeper.h"),
        include_header!("../cpp/bridge/variableview.h"),
    ];

    for &(file_contents, file_name) in header_files {
        let out_path = header_dir.join(file_name);
        let mut header = File::create(out_path).expect("Could not create header");
        header
            .write_all(file_contents)
            .expect("Could not write header");
    }

    CxxQtBuilder::new()
        .include_dir(&header_dir)
        .files([
            "src/ffi/api_code.rs",
            "src/ffi/audio.rs",
            "src/ffi/client.rs",
            "src/ffi/document.rs",
            "src/ffi/plugin_details.rs",
            "src/ffi/regex.rs",
            "src/ffi/sender.rs",
            "src/ffi/sender_map.rs",
            "src/ffi/send_request.rs",
            "src/ffi/timekeeper.rs",
            "src/ffi/util.rs",
            "src/ffi/world.rs",
            "src/ffi/variable_view.rs",
        ])
        .build();
}
