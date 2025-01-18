use crate::modeled::PluginDetailsRust;

#[cxx_qt::bridge]
pub mod ffi {
    unsafe extern "C++" {
        include!("cxx-qt-lib/qstring.h");
        type QString = cxx_qt_lib::QString;
    }

    unsafe extern "C++" {
        include!("cxx-qt-lib/qdate.h");
        type QDate = cxx_qt_lib::QDate;
    }

    unsafe extern "C++" {
        include!("forward.h");
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

    impl
        cxx_qt::Constructor<
            (*const SmushClient, QString),
            NewArguments = (*const SmushClient, QString),
        > for PluginDetails
    {
    }
}
