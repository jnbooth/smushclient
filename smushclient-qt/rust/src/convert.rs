use cxx::kind::Trivial;
use cxx::ExternType;
use cxx_qt_lib::{QColor, QList, QString, QStringList, QVector};
use mud_transformer::mxp::RgbColor;

macro_rules! impl_convert_enum {
    ($ffi:ty, $rust:path, $($variant:ident),+ $(,)?) => {
        impl From<$rust> for $ffi {
            fn from(value: $rust) -> Self {
                match value {
                    $(<$rust>::$variant => Self::$variant),+
                }
            }
        }
        impl From<$ffi> for $rust {
            fn from(value: $ffi) -> Self {
                match value {
                    $(<$ffi>::$variant => Self::$variant),+,
                    #[allow(unreachable_patterns)]
                    _ => unreachable!()
                }
            }
        }
    }
}
macro_rules! impl_convert_enum_opt {
    ($ffi:ty, $rust:path, $fallback:ident, $($variant:ident),+ $(,)?) => {
        impl From<$rust> for $ffi {
            fn from(value: $rust) -> Self {
                match value {
                    $(<$rust>::$variant => Self::$variant),+
                }
            }
        }
        impl From<Option<$rust>> for $ffi {
            fn from(value: Option<$rust>) -> Self {
                match value {
                    None => Self::$fallback,
                    Some(value) => <$ffi>::from(value),
                }
            }
        }
        impl From<$ffi> for Option<$rust> {
            fn from(value: $ffi) -> Self {
                match value {
                    $(<$ffi>::$variant => Some(<$rust>::$variant)),+,
                    _ => None,
                }
            }
        }
        impl Default for $ffi {
            fn default() -> Self {
                Self::$fallback
            }
        }
    }
}

macro_rules! binding {
    ($f:ident, $t:ty) => {
        #[derive(Debug)]
        #[repr(transparent)]
        pub struct $f {
            inner: $t,
        }

        impl From<&$t> for &$f {
            fn from(inner: &$t) -> Self {
                // SAFETY: #[repr(transparent)]
                unsafe { &*(inner as *const $t).cast::<Self>() }
            }
        }
    };
}

pub trait Convert<T> {
    fn convert(&self) -> T;
}

impl Convert<RgbColor> for QColor {
    fn convert(&self) -> RgbColor {
        let rgb = self.to_rgb();
        RgbColor {
            r: u8::try_from(rgb.red()).unwrap(),
            g: u8::try_from(rgb.green()).unwrap(),
            b: u8::try_from(rgb.blue()).unwrap(),
        }
    }
}

impl Convert<Option<RgbColor>> for QColor {
    fn convert(&self) -> Option<RgbColor> {
        if self.alpha() == 0 || !self.is_valid() {
            return None;
        }
        Some(self.convert())
    }
}

impl Convert<QColor> for RgbColor {
    fn convert(&self) -> QColor {
        QColor::from_rgb(i32::from(self.r), i32::from(self.g), i32::from(self.b))
    }
}

impl Convert<QColor> for Option<RgbColor> {
    fn convert(&self) -> QColor {
        match self {
            Some(color) => color.convert(),
            None => QColor::from_rgba(0, 0, 0, 0),
        }
    }
}

impl Convert<Option<String>> for QString {
    fn convert(&self) -> Option<String> {
        if self.is_empty() {
            None
        } else {
            Some(String::from(self))
        }
    }
}

impl Convert<QString> for Option<String> {
    fn convert(&self) -> QString {
        match self {
            Some(s) => QString::from(s),
            None => QString::default(),
        }
    }
}

impl Convert<QStringList> for [String] {
    fn convert(&self) -> QStringList {
        let mut list = QList::default();
        list.reserve(isize::try_from(self.len()).unwrap());
        for item in self {
            list.append(QString::from(item));
        }
        QStringList::from(&list)
    }
}

impl Convert<Vec<String>> for QStringList {
    fn convert(&self) -> Vec<String> {
        QList::from(self).iter().map(String::from).collect()
    }
}

impl<const N: usize> Convert<[String; N]> for QStringList {
    fn convert(&self) -> [String; N] {
        let vec: Vec<String> = self.convert();
        match vec.try_into() {
            Ok(array) => array,
            Err(e) => panic!("expected length {N}, got length {}", e.len()),
        }
    }
}

impl<T, U> Convert<Vec<T>> for QVector<U>
where
    U: cxx_qt_lib::QVectorElement + Convert<T>,
{
    fn convert(&self) -> Vec<T> {
        self.iter().map(Convert::convert).collect()
    }
}

impl<T, U, const N: usize> Convert<[T; N]> for QVector<U>
where
    U: cxx_qt_lib::QVectorElement + Convert<T>,
{
    fn convert(&self) -> [T; N] {
        let vec: Vec<T> = self.convert();
        match vec.try_into() {
            Ok(array) => array,
            Err(e) => panic!("expected length {N}, got length {}", e.len()),
        }
    }
}

impl<T, U> Convert<QVector<T>> for [U]
where
    T: ExternType<Kind = Trivial> + cxx_qt_lib::QVectorElement,
    U: Convert<T>,
{
    fn convert(&self) -> QVector<T> {
        let mut qvec = QVector::default();
        qvec.reserve(isize::try_from(self.len()).unwrap());
        for item in self {
            qvec.append(item.convert());
        }
        qvec
    }
}
