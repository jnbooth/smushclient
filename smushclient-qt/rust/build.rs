use std::env;
use std::fs::{self, File};
use std::io::Write;

use cxx_qt_build::{CxxQtBuilder, QmlModule};

fn main() {
    let header_root = match env::var("CXXQT_EXPORT_DIR") {
        Ok(export_dir) => format!("{export_dir}/{}", env::var("CARGO_PKG_NAME").unwrap()),
        Err(_) => env::var("OUT_DIR").unwrap(),
    };
    cxx_qt_extensions_headers::write_headers(format!("{header_root}/cxx-qt-extensions"));

    let header_dir = format!("{header_root}/cxx-qt-gen");
    fs::create_dir_all(&header_dir).expect("Could not create header dir");

    for (file_contents, file_name) in [
        (include_str!("../cpp/bridge/document.h"), "document.h"),
        (include_str!("../cpp/bridge/timekeeper.h"), "timekeeper.h"),
        (include_str!("../cpp/bridge/viewbuilder.h"), "viewbuilder.h"),
    ] {
        let h_path = format!("{header_dir}/{file_name}");
        let mut header = File::create(h_path).expect("Could not create cxx-qt-gen header");
        write!(header, "{file_contents}").expect("Could not write cxx-qt-gen header");
    }

    CxxQtBuilder::new()
        .qml_module::<&str, &str>(QmlModule {
            uri: "com.jnbooth.smushclient",
            rust_files: &["src/lib.rs"],
            qml_files: &[],
            ..Default::default()
        })
        .build();
}
