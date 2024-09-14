use std::env;
use std::fs::{self, File};
use std::io::Write;

use cxx_qt_build::{CxxQtBuilder, QmlModule};

fn get_header_dir() -> String {
    let header_root = match env::var("CXXQT_EXPORT_DIR") {
        Ok(export_dir) => format!("{export_dir}/{}", env::var("CARGO_PKG_NAME").unwrap()),
        Err(_) => env::var("OUT_DIR").unwrap(),
    };
    format!("{header_root}/cxx-qt-gen")
}

fn copy_header(dir: &str, name: &str, contents: &str) {
    let path = format!("{dir}/{name}");
    let mut file = File::create(path).expect("Could not create {name}");
    write!(file, "{contents}").expect("Could not write {name}");
}

fn main() {
    let header_dir = get_header_dir();
    fs::create_dir_all(&header_dir).expect("Could not create header dir");
    copy_header(
        &header_dir,
        "document.h",
        include_str!("../cpp/bridge/document.h"),
    );
    copy_header(
        &header_dir,
        "scriptengine.h",
        include_str!("../cpp/bridge/scriptengine.h"),
    );
    copy_header(
        &header_dir,
        "treebuilder.h",
        include_str!("../cpp/bridge/treebuilder.h"),
    );

    CxxQtBuilder::new()
        .qml_module::<&str, &str>(QmlModule {
            uri: "com.jnbooth.smushclient",
            rust_files: &["src/lib.rs"],
            qml_files: &[],
            ..Default::default()
        })
        .build();
}
