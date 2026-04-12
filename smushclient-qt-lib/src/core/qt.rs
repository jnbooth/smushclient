#[cxx::bridge(namespace = "Qt")]
mod ffi {
    #[repr(i32)]
    enum BrushStyle {
        NoBrush,
        SolidPattern,
        Dense1Pattern,
        Dense2Pattern,
        Dense3Pattern,
        Dense4Pattern,
        Dense5Pattern,
        Dense6Pattern,
        Dense7Pattern,
        HorPattern,
        VerPattern,
        CrossPattern,
        BDiagPattern,
        FDiagPattern,
        DiagCrossPattern,
        LinearGradientPattern,
        RadialGradientPattern,
        ConicalGradientPattern,
        TexturePattern = 24,
    }

    #[repr(i32)]
    enum GlobalColor {
        color0,
        color1,
        black,
        white,
        darkGray,
        gray,
        lightGray,
        red,
        green,
        blue,
        cyan,
        magenta,
        yellow,
        darkRed,
        darkGreen,
        darkBlue,
        darkCyan,
        darkMagenta,
        darkYellow,
        transparent,
    }

    #[repr(i32)]
    enum TextElideMode {
        /// The ellipsis should appear at the beginning of the text.
        ElideLeft,
        /// The ellipsis should appear at the end of the text.
        ElideRight,
        /// The ellipsis should appear in the middle of the text.
        ElideMiddle,
        /// Ellipsis should NOT appear in the text. When passed to functions such as [`QFontMetricsF::elided_text`](crate::QFontMetricsF::elided_text), this will cause the full string to return unless the text contains multi-length variants. Elision in this case must be done by clipping to the component width.
        ElideNone,
    }
}

pub use ffi::{BrushStyle, GlobalColor, TextElideMode};
