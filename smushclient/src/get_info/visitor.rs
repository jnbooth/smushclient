use smushclient_plugins::NaiveDate;

pub trait InfoVisitor {
    type Output;

    fn visit_i16(info: i16) -> Self::Output;
    fn visit_bool(info: bool) -> Self::Output;
    fn visit_date(info: NaiveDate) -> Self::Output;
    fn visit_double(info: f64) -> Self::Output;
    fn visit_none() -> Self::Output;
    fn visit_str(info: &str) -> Self::Output;
    fn visit_usize(info: usize) -> Self::Output;
}