use std::path::Path;

use chrono::{DateTime, NaiveDate, Utc};
use cxx_qt_lib::{QByteArray, QDate, QDateTime, QString, QVariant};
use mud_transformer::mxp::RgbColor;
use smushclient::InfoVisitor;
use smushclient_plugins::SendTarget;

use crate::convert::Convert;
use crate::ffi;

pub struct InfoVisitorQVariant;

macro_rules! impl_visit {
    ($i:ident, $t:ty) => {
        fn $i(info: $t) -> Self::Output {
            QVariant::from(&info)
        }
    };
}

impl InfoVisitor for InfoVisitorQVariant {
    type Output = QVariant;

    impl_visit!(visit_bool, bool);
    impl_visit!(visit_double, f64);
    impl_visit!(visit_float, f32);
    impl_visit!(visit_i8, i8);
    impl_visit!(visit_i16, i16);
    impl_visit!(visit_i32, i32);
    impl_visit!(visit_i64, i64);
    impl_visit!(visit_u8, u8);
    impl_visit!(visit_u16, u16);
    impl_visit!(visit_u32, u32);
    impl_visit!(visit_u64, u64);

    fn visit_color(info: RgbColor) -> Self::Output {
        QVariant::from(&info.convert())
    }

    fn visit_date(info: NaiveDate) -> Self::Output {
        QVariant::from(&QDate::from(info))
    }

    fn visit_datetime(info: DateTime<Utc>) -> Self::Output {
        match QDateTime::try_from(info) {
            Ok(datetime) => QVariant::from(&datetime),
            Err(_) => QVariant::default(),
        }
    }

    fn visit_none() -> Self::Output {
        QVariant::default()
    }

    fn visit_path(info: &Path) -> Self::Output {
        QVariant::from(&QByteArray::from(info.as_os_str().as_encoded_bytes()))
    }

    fn visit_send_target(info: SendTarget) -> Self::Output {
        QVariant::from(&ffi::SendTarget::from(info).repr)
    }

    fn visit_str(info: &str) -> Self::Output {
        QVariant::from(&QString::from(info))
    }
}
