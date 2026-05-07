use std::fmt;
use std::mem::MaybeUninit;

use cxx::{ExternType, type_id};
use cxx_qt_lib::{QColor, QImage};

use crate::util::new_in_place;
use crate::{BrushStyle, GlobalColor};

#[cxx::bridge]
mod ffi {
    extern "C++" {
        include!("cxx-qt-lib/qstring.h");
        type QString = cxx_qt_lib::QString;

        include!("cxx-qt-lib/qcolor.h");
        type QColor = cxx_qt_lib::QColor;
        include!("cxx-qt-lib/qimage.h");
        type QImage = cxx_qt_lib::QImage;
    }

    #[namespace = "Qt"]
    extern "C++" {
        include!("cxx-qt-lib/qt.h");
        type BrushStyle = crate::BrushStyle;
        type GlobalColor = crate::GlobalColor;
    }

    extern "C++" {
        include!("smushclient-qt-lib/qbrush.h");
    }

    unsafe extern "C++" {
        type QBrush = super::QBrush;

        /// Returns the brush color.
        fn color(&self) -> &QColor;

        /// Returns `true` if the brush is fully opaque otherwise `false`.
        #[rust_name = "is_opaque"]
        fn isOpaque(&self) -> bool;

        #[doc(hidden)]
        #[rust_name = "set_color_qcolor"]
        fn setColor(&mut self, color: &QColor);

        #[doc(hidden)]
        #[rust_name = "set_color_globalcolor"]
        fn setColor(&mut self, color: GlobalColor);

        /// Sets the brush style to `style`.
        #[rust_name = "set_style"]
        fn setStyle(&mut self, style: BrushStyle);

        /// Sets the brush image to `image`. The style is set to [`BrushStyle::TexturePattern`].
        #[rust_name = "set_texture_image"]
        fn setTextureImage(&mut self, image: &QImage);

        /// Returns the brush style.
        fn style(&self) -> BrushStyle;

        /// Swaps this brush with `other`. This function is very fast and never fails.
        fn swap(&mut self, other: &mut QBrush);

        #[doc(hidden)]
        #[rust_name = "texture_image_or_null"]
        fn textureImage(&self) -> QImage;
    }

    #[namespace = "rust::cxxqtlib1"]
    unsafe extern "C++" {
        include!("cxx-qt-lib/common.h");

        #[rust_name = "qbrush_drop"]
        fn drop(config: &mut QBrush);

        #[rust_name = "qbrush_eq"]
        fn operatorEq(a: &QBrush, b: &QBrush) -> bool;

        #[rust_name = "qbrush_to_debug_qstring"]
        fn toDebugQString(value: &QBrush) -> QString;
    }

    #[namespace = "rust::cxxqtio1"]
    unsafe extern "C++" {
        include!("cxx-qt-io/common.h");

        #[rust_name = "qbrush_init_default"]
        unsafe fn constructInPlace(uninit: *mut QBrush);
        #[rust_name = "qbrush_clone"]
        unsafe fn constructInPlace(uninit: *mut QBrush, other: &QBrush);
        #[rust_name = "qbrush_init_brushstyle"]
        unsafe fn constructInPlace(uninit: *mut QBrush, style: BrushStyle);
        #[rust_name = "qbrush_init_qimage"]
        unsafe fn constructInPlace(uninit: *mut QBrush, image: &QImage);
        #[rust_name = "qbrush_init_globalcolor"]
        unsafe fn constructInPlace(uninit: *mut QBrush, color: GlobalColor, style: BrushStyle);
        #[rust_name = "qbrush_init_qcolor"]
        unsafe fn constructInPlace(uninit: *mut QBrush, color: &QColor, style: BrushStyle);
    }
}

/// The `QBrush` class defines the fill pattern of shapes drawn by [`QPainter`](cxx_qt_lib::QPainter).
///
/// Qt Documentation: [QBrush](https://doc.qt.io/qt-6/qbrush.html#details)
#[repr(C)]
pub struct QBrush {
    _space: MaybeUninit<usize>,
}

impl Clone for QBrush {
    fn clone(&self) -> Self {
        // SAFETY: ffi:: initializes the passed pointer in-place.
        unsafe { new_in_place(|p| ffi::qbrush_clone(p, self)) }
    }
}

impl Default for QBrush {
    /// Constructs a default black brush with the style [`BrushStyle::NoBrush`] (i.e. this brush will not fill shapes).
    fn default() -> Self {
        // SAFETY: ffi:: initializes the passed pointer in-place.
        unsafe { new_in_place(|p| ffi::qbrush_init_default(p)) }
    }
}

impl Drop for QBrush {
    fn drop(&mut self) {
        ffi::qbrush_drop(self);
    }
}

impl PartialEq for QBrush {
    fn eq(&self, other: &Self) -> bool {
        ffi::qbrush_eq(self, other)
    }
}

impl Eq for QBrush {}

impl fmt::Debug for QBrush {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        ffi::qbrush_to_debug_qstring(self).fmt(f)
    }
}

impl From<BrushStyle> for QBrush {
    /// Constructs a black brush with the given `style`.
    fn from(style: BrushStyle) -> Self {
        // SAFETY: ffi:: initializes the passed pointer in-place.
        unsafe { new_in_place(|p| ffi::qbrush_init_brushstyle(p, style)) }
    }
}

impl From<&QImage> for QBrush {
    /// Constructs a brush with a black color and a texture set to the given `image`. The style is set to [`BrushStyle::TexturePattern`].
    fn from(image: &QImage) -> Self {
        // SAFETY: ffi:: initializes the passed pointer in-place.
        unsafe { new_in_place(|p| ffi::qbrush_init_qimage(p, image)) }
    }
}

impl From<GlobalColor> for QBrush {
    /// Constructs a brush with the given `color`.
    fn from(color: GlobalColor) -> Self {
        // SAFETY: ffi:: initializes the passed pointer in-place.
        unsafe {
            new_in_place(|p| ffi::qbrush_init_globalcolor(p, color, BrushStyle::SolidPattern))
        }
    }
}

impl From<&QColor> for QBrush {
    /// Constructs a brush with the given `color`.
    fn from(color: &QColor) -> Self {
        // SAFETY: ffi:: initializes the passed pointer in-place.
        unsafe { new_in_place(|p| ffi::qbrush_init_qcolor(p, color, BrushStyle::SolidPattern)) }
    }
}

impl QBrush {
    /// Constructs a brush with the given `color` and `style`.
    pub fn new<T: QBrushColor>(color: T, style: BrushStyle) -> Self {
        color.new_brush(style)
    }

    /// Sets the brush color to the given `color`.
    pub fn set_color<T: QBrushColor>(&mut self, color: T) {
        color.set_color(self);
    }

    /// Returns the custom brush pattern, or `None` if no custom brush pattern has been set.
    pub fn texture_image(&self) -> Option<QImage> {
        let image = self.texture_image_or_null();
        if image.is_null() { None } else { Some(image) }
    }
}

// SAFETY: Static checks on the C++ side to ensure the size is the same.
unsafe impl ExternType for QBrush {
    type Id = type_id!("QBrush");
    type Kind = cxx::kind::Trivial;
}

mod private {
    pub trait Sealed {}
}

/// A [`QBrush`]'s color can be specified by a [`GlobalColor`] or a [`QColor`].
pub trait QBrushColor: private::Sealed {
    fn new_brush(self, style: BrushStyle) -> QBrush;
    fn set_color(self, brush: &mut QBrush);
}

impl private::Sealed for GlobalColor {}

impl QBrushColor for GlobalColor {
    fn new_brush(self, style: BrushStyle) -> QBrush {
        // SAFETY: ffi:: initializes the passed pointer in-place.
        unsafe { new_in_place(|p| ffi::qbrush_init_globalcolor(p, self, style)) }
    }

    fn set_color(self, brush: &mut QBrush) {
        brush.set_color_globalcolor(self);
    }
}

impl private::Sealed for &QColor {}

impl QBrushColor for &QColor {
    fn new_brush(self, style: BrushStyle) -> QBrush {
        // SAFETY: ffi:: initializes the passed pointer in-place.
        unsafe { new_in_place(|p| ffi::qbrush_init_qcolor(p, self, style)) }
    }

    fn set_color(self, brush: &mut QBrush) {
        brush.set_color_qcolor(self);
    }
}
