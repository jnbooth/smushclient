use std::path::Path;

use chrono::{DateTime, NaiveDate, Utc};
use cxx_qt_lib::{QByteArray, QDate, QDateTime, QFont, QString, QVariant};
use mud_transformer::mxp::RgbColor;
use smushclient::InfoVisitor;
use smushclient_plugins::SendTarget;
use smushclient_qt_lib::{QChar, QFontInfo, QFontMetrics};

use crate::convert::Convert;
use crate::ffi;

pub struct InfoVisitorQVariant;

macro_rules! impl_visit {
    ($i:ident, $t:ty) => {
        fn $i(info: $t) -> Self::Output {
            QVariant::from(&info)
        }
    };

    ($i:ident, $t:ty, $f:ty) => {
        fn $i(info: $t) -> Self::Output {
            QVariant::from(&<$f>::from(info))
        }
    };
}

impl InfoVisitor for InfoVisitorQVariant {
    type Output = QVariant;

    impl_visit!(visit_bool, bool);
    impl_visit!(visit_bytes, &[u8], QByteArray);
    impl_visit!(visit_date, NaiveDate, QDate);
    impl_visit!(visit_double, f64);
    impl_visit!(visit_float, f32);
    impl_visit!(visit_str, &str, QString);
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
}

pub fn font_info(font: &QFont, info_type: i64) -> QVariant {
    const OVERHANG_SAMPLE: QChar = QChar::from_char('l').unwrap();

    macro_rules! info {
        ($i:ident) => {
            (&QFontInfo::new(font).$i()).into()
        };
    }
    macro_rules! metric {
        ($i:ident) => {
            (&QFontMetrics::new(font).$i()).into()
        };
    }
    match info_type {
        1 => metric!(height),
        2 => metric!(ascent),
        3 => metric!(descent),
        4 => metric!(leading),
        5 => (&0).into(), // external leading
        6 => metric!(average_char_width),
        7 => metric!(max_width),
        8 => info!(weight),
        9 => (&QFontMetrics::new(font).left_bearing(OVERHANG_SAMPLE)).into(),
        // 10 => digitized aspect X
        // 11 => digitized aspect Y
        // 12 => first character defined in font
        // 13 => last character defined in font
        14 => (&QString::from("�")).into(),
        // 15 => character used to define word breaks
        16 => info!(italic),
        17 => info!(underline),
        18 => info!(strike_out),
        // 19 => pitch and family (handled by frontend)
        // 20 => character set
        21 => info!(family),
        _ => QVariant::default(),
    }
}
