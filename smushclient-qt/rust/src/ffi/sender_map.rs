use crate::modeled::SenderMapRust;

#[cxx_qt::bridge]
pub mod ffi {
    unsafe extern "C++" {
        include!("cxx-qt-lib/qset.h");
        type QSet_u16 = cxx_qt_lib::QSet<u16>;
    }

    unsafe extern "C++" {
        include!("cxx-qt-lib/qstring.h");
        type QString = cxx_qt_lib::QString;
    }

    unsafe extern "C++" {
        include!("cxx-qt-lib/qvariant.h");
        type QVariant = cxx_qt_lib::QVariant;
    }

    unsafe extern "C++" {
        include!("forward.h");
        #[cxx_name = "SmushClientBase"]
        type SmushClient = crate::ffi::SmushClient;
    }

    #[qenum(SenderMap)]
    enum SenderType {
        Alias,
        Timer,
        Trigger,
    }

    extern "RustQt" {
        #[qobject]
        type SenderMap = super::SenderMapRust;
    }

    #[auto_cxx_name]
    unsafe extern "RustQt" {
        pub fn cell_text(
            self: &SenderMap,
            client: &SmushClient,
            group: &String,
            index: usize,
            column: i32,
        ) -> QString;
        fn len(self: &SenderMap) -> usize;
        fn group_len(self: &SenderMap, group_index: usize) -> usize;
        fn group_index(self: &SenderMap, group: &String) -> i32;
        fn group_name(self: &SenderMap, group_index: usize) -> *const String;
        fn position_in_group(self: &SenderMap, group: &String, index: usize) -> i32;
        fn recalculate(self: Pin<&mut SenderMap>, client: &SmushClient);
        fn remove(
            self: &SenderMap,
            client: Pin<&mut SmushClient>,
            group: &String,
            first: usize,
            amount: usize,
        ) -> bool;
        fn sender_index(self: &SenderMap, group: &String, index: usize) -> i32;
        fn set_cell(
            self: Pin<&mut SenderMap>,
            client: Pin<&mut SmushClient>,
            group: &String,
            index: usize,
            column: i32,
            data: &QVariant,
        ) -> i32;
        fn timer_ids(
            self: &SenderMap,
            client: &SmushClient,
            group: &String,
            first: usize,
            amount: usize,
        ) -> QSet_u16;
    }

    impl cxx_qt::Constructor<(SenderType,), NewArguments = (SenderType,)> for SenderMap {}
}
