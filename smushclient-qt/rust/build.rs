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

fn copy_document_header(header_dir: &str) {
    let path = format!("{header_dir}/document.h");
    let mut file = File::create(path).expect("Could not create document.h");
    let contents = include_str!("../cpp/document.h");
    write!(file, "{contents}").expect("Could not write document.h");
}

fn main() {
    let header_dir = get_header_dir();
    fs::create_dir_all(&header_dir).expect("Could not create header dir");
    copy_document_header(&header_dir);

    CxxQtBuilder::new()
        .qml_module::<&str, &str>(QmlModule {
            uri: "com.jnbooth.smushclient",
            rust_files: &["src/lib.rs"],
            qml_files: &[],
            ..Default::default()
        })
        .build();
}
