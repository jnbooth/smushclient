use crate::modeled::PluginDetailsRust;

#[cxx_qt::bridge]
pub mod ffi {
    extern "C++" {
        include!("cxx-qt-lib/qstring.h");
        type QString = cxx_qt_lib::QString;
        include!("cxx-qt-lib/qdate.h");
        type QDate = cxx_qt_lib::QDate;
    }

    extern "C++" {
        include!("smushclient_qt/src/ffi/client.cxxqt.h");
        #[cxx_name = "SmushClientBase"]
        type SmushClient = crate::ffi::SmushClient;
    }

    extern "RustQt" {
        #[qobject]
        #[qproperty(QString, name)]
        #[qproperty(QString, version)]
        #[qproperty(QString, author)]
        #[qproperty(QDate, written)]
        #[qproperty(QDate, modified)]
        #[qproperty(QString, id)]
        #[qproperty(QString, file)]
        #[qproperty(QString, description)]
        type PluginDetails = super::PluginDetailsRust;
    }

    impl cxx_qt::Constructor<(), NewArguments = ()> for PluginDetails {}

    impl<'a>
        cxx_qt::Constructor<(&'a SmushClient, QString), NewArguments = (&'a SmushClient, QString)>
        for PluginDetails
    {
    }

    impl<'a> cxx_qt::Constructor<(&'a SmushClient, usize), NewArguments = (&'a SmushClient, usize)>
        for PluginDetails
    {
    }
}
