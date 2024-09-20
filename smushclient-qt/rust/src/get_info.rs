use cxx_qt_lib::{QDate, QString, QVariant};
use smushclient::InfoVisitor;
use smushclient_plugins::NaiveDate;

pub struct InfoVisitorQVariant;

impl InfoVisitor for InfoVisitorQVariant {
    type Output = QVariant;

    fn visit_bool(info: bool) -> Self::Output {
        QVariant::from(&info)
    }

    fn visit_date(info: NaiveDate) -> Self::Output {
        QVariant::from(&QDate::from(info))
    }

    fn visit_double(info: f64) -> Self::Output {
        QVariant::from(&info)
    }

    fn visit_i16(info: i16) -> Self::Output {
        QVariant::from(&info)
    }

    fn visit_none() -> Self::Output {
        QVariant::default()
    }

    fn visit_str(info: &str) -> Self::Output {
        QVariant::from(&QString::from(info))
    }

    fn visit_usize(info: usize) -> Self::Output {
        QVariant::from(&u64::try_from(info).unwrap())
    }
}