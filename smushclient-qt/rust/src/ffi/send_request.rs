#[cxx::bridge]
pub mod ffi {
    extern "C++" {
        include!("cxx-qt-lib/qstring.h");
        type QString = cxx_qt_lib::QString;
    }

    enum SenderKind {
        Trigger,
        Alias,
        Timer,
    }

    #[repr(i32)]
    enum SendTarget {
        World,
        Command,
        Output,
        Status,
        NotepadNew,
        NotepadAppend,
        Log,
        NotepadReplace,
        WorldDelay,
        Variable,
        Execute,
        Speedwalk,
        Script,
        WorldImmediate,
        ScriptAfterOmit,
    }

    struct SendRequest {
        plugin: usize,
        #[cxx_name = "sendTo"]
        send_to: SendTarget,
        echo: bool,
        log: bool,
        text: QString,
        destination: QString,
    }

    struct SendTimer {
        #[cxx_name = "activeClosed"]
        active_closed: bool,
        label: String,
        script: String,
        request: SendRequest,
    }
}
