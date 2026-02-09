use cxx_qt_build::CxxQtBuilder;

fn main() {
    CxxQtBuilder::new()
        .crate_include_root(Some("../cpp/bridge".to_owned()))
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
            "src/ffi/spans.rs",
            "src/ffi/timekeeper.rs",
            "src/ffi/util.rs",
            "src/ffi/world.rs",
            "src/ffi/variable_view.rs",
        ])
        .build();
}
