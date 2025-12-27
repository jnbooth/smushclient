use std::ffi::OsStr;
use std::fs::File;
use std::io::Write;
use std::path::{Path, PathBuf};
use std::{env, fs};

use cxx_qt_build::CxxQtBuilder;

macro_rules! include_header {
    ($file:literal) => {
        (include_bytes!($file), Path::new($file).file_name().unwrap())
    };
}

struct HeaderBuilder {
    dir: PathBuf,
}

impl HeaderBuilder {
    pub fn new() -> Self {
        let dir = PathBuf::from(env::var("OUT_DIR").unwrap())
            .join("include")
            .join("smushclient-qt-lib");
        Self { dir }
    }

    pub fn create_header_dir(&self) {
        fs::create_dir_all(&self.dir).expect("Failed to create include directory");
    }

    pub fn write_headers(&self, files: &[(&[u8], &OsStr)]) {
        for &(file_contents, file_name) in files {
            let out_path = self.dir.join(file_name);
            let mut header = File::create(out_path).expect("Could not create header");
            header
                .write_all(file_contents)
                .expect("Could not write header");
        }
    }
}

impl AsRef<Path> for HeaderBuilder {
    fn as_ref(&self) -> &Path {
        &self.dir
    }
}

fn main() {
    let header_dir = HeaderBuilder::new();
    header_dir.create_header_dir();
    header_dir.write_headers(&[
        include_header!("include/core/qlist/qlist_private.h"),
        include_header!("include/core/qlist/qlist_qtextlength.h"),
        include_header!("include/core/qlist/qlist.h"),
        include_header!("include/core/qmap/qmap_i32_qvariant.h"),
        include_header!("include/core/qmap/qmap_private.h"),
        include_header!("include/core/qmap/qmap.h"),
        include_header!("include/core/qvariant/qvariant.h"),
        include_header!("include/core/qvariant/qvariant_qbrush.h"),
        include_header!("include/core/qvariant/qvariant_qtextformat.h"),
        include_header!("include/core/qvariant/qvariant_qtextlength.h"),
        include_header!("include/gui/qbrush.h"),
        include_header!("include/gui/qtextcharformat.h"),
        include_header!("include/gui/qtextformat.h"),
        include_header!("include/gui/qtextlength.h"),
    ]);

    let cpp_files = &[
        "src/core/qlist/qlist.cpp",
        "src/core/qmap/qmap.cpp",
        "src/core/qvariant/qvariant.cpp",
        "src/gui/qbrush.cpp",
        "src/gui/qtextcharformat.cpp",
        "src/gui/qtextformat.cpp",
        "src/gui/qtextlength.cpp",
    ];

    let rust_files = &[
        "src/core/qmap/qmap_i32_qvariant.rs",
        "src/core/qlist/qlist_qtextlength.rs",
        "src/core/qt.rs",
        "src/core/qvariant/qvariant_qbrush.rs",
        "src/core/qvariant/qvariant_qtextformat.rs",
        "src/core/qvariant/qvariant_qtextlength.rs",
        "src/gui/qbrush.rs",
        "src/gui/qtextcharformat.rs",
        "src/gui/qtextformat.rs",
        "src/gui/qtextlength.rs",
    ];

    CxxQtBuilder::new()
        .include_prefix("private")
        .crate_include_root(Some("include/".to_owned()))
        .include_dir(&header_dir)
        .qt_module("Gui")
        .files(rust_files)
        .cc_builder(move |cc| {
            for cpp_file in cpp_files {
                cc.file(cpp_file);
                println!("cargo::rerun-if-changed={cpp_file}");
            }
        })
        .build()
        .reexport_dependency("cxx-qt-lib")
        .export();
}
