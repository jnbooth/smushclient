use std::path::{Path, PathBuf};

use chrono::{DateTime, NaiveDate, Utc};
use mud_transformer::mxp::RgbColor;
use smushclient_plugins::SendTarget;

pub trait InfoVisitor {
    type Output;

    fn visit_bool(info: bool) -> Self::Output;
    fn visit_bytes(info: &[u8]) -> Self::Output;
    fn visit_color(info: RgbColor) -> Self::Output;
    fn visit_date(info: NaiveDate) -> Self::Output;
    fn visit_datetime(info: DateTime<Utc>) -> Self::Output;
    fn visit_double(info: f64) -> Self::Output;
    fn visit_i64(info: i64) -> Self::Output;
    fn visit_none() -> Self::Output;
    fn visit_path(info: &Path) -> Self::Output;
    fn visit_send_target(info: SendTarget) -> Self::Output;
    fn visit_str(info: &str) -> Self::Output;
    fn visit_u64(info: u64) -> Self::Output;

    fn visit_float(info: f32) -> Self::Output {
        Self::visit_double(info.into())
    }

    fn visit_i8(info: i8) -> Self::Output {
        Self::visit_i64(info.into())
    }
    fn visit_i16(info: i16) -> Self::Output {
        Self::visit_i64(info.into())
    }
    fn visit_i32(info: i32) -> Self::Output {
        Self::visit_i64(info.into())
    }
    fn visit_isize(info: isize) -> Self::Output {
        Self::visit_i64(match i64::try_from(info) {
            Ok(info) => info,
            Err(_) if info >= 0 => i64::MAX,
            Err(_) => i64::MIN,
        })
    }

    fn visit_u8(info: u8) -> Self::Output {
        Self::visit_u64(info.into())
    }
    fn visit_u16(info: u16) -> Self::Output {
        Self::visit_u64(info.into())
    }
    fn visit_u32(info: u32) -> Self::Output {
        Self::visit_u64(info.into())
    }
    fn visit_usize(info: usize) -> Self::Output {
        Self::visit_u64(info.try_into().unwrap_or(u64::MAX))
    }

    fn visit<I: VisitorInfo>(info: I) -> Self::Output {
        info.visit::<Self>()
    }
}

pub trait VisitorInfo {
    fn visit<V: InfoVisitor + ?Sized>(self) -> V::Output;
}

impl<I: VisitorInfo> VisitorInfo for Option<I> {
    fn visit<V: InfoVisitor + ?Sized>(self) -> V::Output {
        match self {
            Some(info) => info.visit::<V>(),
            None => V::visit_none(),
        }
    }
}

macro_rules! impl_info {
    ($t:ty, $i:tt) => {
        impl VisitorInfo for $t {
            fn visit<V: InfoVisitor + ?Sized>(self) -> V::Output {
                V::$i(self)
            }
        }
    };
}

impl_info!(bool, visit_bool);
impl_info!(&[u8], visit_bytes);
impl_info!(RgbColor, visit_color);
impl_info!(NaiveDate, visit_date);
impl_info!(DateTime<Utc>, visit_datetime);
impl_info!(f64, visit_double);
impl_info!(i64, visit_i64);
impl_info!(&Path, visit_path);
impl_info!(&PathBuf, visit_path);
impl_info!(SendTarget, visit_send_target);
impl_info!(&str, visit_str);
impl_info!(&String, visit_str);
impl_info!(u64, visit_u64);
impl_info!(f32, visit_float);
impl_info!(i8, visit_i8);
impl_info!(i16, visit_i16);
impl_info!(i32, visit_i32);
impl_info!(isize, visit_isize);
impl_info!(u8, visit_u8);
impl_info!(u16, visit_u16);
impl_info!(u32, visit_u32);
impl_info!(usize, visit_usize);

impl VisitorInfo for &Option<String> {
    fn visit<V: InfoVisitor + ?Sized>(self) -> V::Output {
        V::visit_str(self.as_deref().unwrap_or_default())
    }
}
