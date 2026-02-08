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

trait CxxQtBuilderExt {
    fn cpp_files(self, cpp_files: &[&str]) -> Self;
}

impl CxxQtBuilderExt for CxxQtBuilder {
    fn cpp_files(mut self, cpp_files: &[&str]) -> Self {
        for cpp_file in cpp_files {
            self = self.cpp_file(cpp_file);
        }
        self
    }
}

#[allow(clippy::needless_borrows_for_generic_args)]
fn main() {
    let header_dir = PathBuf::from(env::var("OUT_DIR").unwrap())
        .join("include")
        .join("smushclient-qt-lib");

    fs::create_dir_all(&header_dir).expect("Failed to create include directory");

    let header_files: &[(&[u8], &OsStr)] = &[
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
        include_header!("include/gui/qtextcursor.h"),
        include_header!("include/gui/qtextformat.h"),
        include_header!("include/gui/qtextlength.h"),
    ];

    for &(file_contents, file_name) in header_files {
        let out_path = header_dir.join(file_name);
        let mut header = File::create(out_path).expect("Could not create header");
        header
            .write_all(file_contents)
            .expect("Could not write header");
    }

    CxxQtBuilder::new()
        .include_prefix("private")
        .crate_include_root(Some("include/".to_owned()))
        .include_dir(&header_dir)
        .qt_module("Gui")
        .cpp_files(&[
            "src/core/qlist/qlist.cpp",
            "src/core/qmap/qmap.cpp",
            "src/core/qvariant/qvariant.cpp",
            "src/gui/qbrush.cpp",
            "src/gui/qtextcharformat.cpp",
            "src/gui/qtextformat.cpp",
            "src/gui/qtextlength.cpp",
        ])
        .files(&[
            "src/core/qmap/qmap_i32_qvariant.rs",
            "src/core/qlist/qlist_qtextlength.rs",
            "src/core/qt.rs",
            "src/core/qvariant/qvariant_qbrush.rs",
            "src/core/qvariant/qvariant_qtextformat.rs",
            "src/core/qvariant/qvariant_qtextlength.rs",
            "src/gui/qbrush.rs",
            "src/gui/qtextcharformat.rs",
            "src/gui/qtextcursor.rs",
            "src/gui/qtextformat.rs",
            "src/gui/qtextlength.rs",
        ])
        .build()
        .reexport_dependency("cxx-qt-lib")
        .export();
}
