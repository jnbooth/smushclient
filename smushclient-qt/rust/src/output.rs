use super::ffi;
use cxx_qt_lib::{QByteArray, QColor, QString};
use mud_transformer::mxp::RgbColor;
use mud_transformer::{
    EffectFragment, EntityFragment, OutputFragment, TelnetFragment, TextFragment, TextStyle,
};

fn encode_color(color: RgbColor) -> QColor {
    QColor::from_rgb(color.r as i32, color.g as i32, color.b as i32)
}

binding!(RustTextFragment, TextFragment);

macro_rules! flag_method {
    ($n:ident, $v:expr) => {
        pub fn $n(&self) -> bool {
            self.inner.flags.contains($v)
        }
    };
}

impl RustTextFragment {
    pub fn text(&self) -> QString {
        QString::from(&*self.inner.text)
    }

    pub fn foreground(&self) -> QColor {
        encode_color(self.inner.foreground)
    }

    pub fn background(&self) -> QColor {
        encode_color(self.inner.background)
    }

    pub fn has_link(&self) -> bool {
        self.inner.action.is_some()
    }

    pub fn link(&self) -> ffi::MxpLink {
        self.inner.action.as_ref().unwrap().into()
    }

    pub fn has_font(&self) -> bool {
        self.inner.font.is_some()
    }

    pub fn font(&self) -> QString {
        match &self.inner.font {
            Some(font) => QString::from(font),
            None => QString::default(),
        }
    }

    pub fn has_size(&self) -> bool {
        self.inner.size.is_some()
    }

    pub fn size(&self) -> u8 {
        match self.inner.size {
            Some(size) => size.get(),
            None => 0,
        }
    }

    pub fn is_heading(&self) -> bool {
        self.inner.heading.is_some()
    }

    pub fn heading(&self) -> ffi::Heading {
        self.inner.heading.into()
    }

    flag_method!(is_blink, TextStyle::Blink);
    flag_method!(is_bold, TextStyle::Bold);
    flag_method!(is_highlight, TextStyle::Highlight);
    flag_method!(is_inverse, TextStyle::Inverse);
    flag_method!(is_italic, TextStyle::Italic);
    flag_method!(is_strikeout, TextStyle::Strikeout);
    flag_method!(is_underline, TextStyle::Underline);
}

binding!(RustEffectFragment, EffectFragment);

binding!(RustTelnetFragment, TelnetFragment);

impl RustTelnetFragment {
    pub fn kind(&self) -> ffi::TelnetRequest {
        match self.inner {
            TelnetFragment::Do { .. } => ffi::TelnetRequest::Do,
            TelnetFragment::IacGa => ffi::TelnetRequest::IacGa,
            TelnetFragment::Naws => ffi::TelnetRequest::Naws,
            TelnetFragment::SetEcho { .. } => todo!(),
            TelnetFragment::Subnegotiation { .. } => ffi::TelnetRequest::Subnegotiation,
            TelnetFragment::Will { .. } => ffi::TelnetRequest::Will,
        }
    }

    pub fn code(&self) -> u8 {
        match self.inner {
            TelnetFragment::Will { code }
            | TelnetFragment::Do { code }
            | TelnetFragment::Subnegotiation { code, .. } => code,
            _ => 0,
        }
    }

    pub fn data(&self) -> QByteArray {
        match &self.inner {
            TelnetFragment::Subnegotiation { data, .. } => QByteArray::from(&**data),
            _ => QByteArray::default(),
        }
    }
}

binding!(RustOutputFragment, OutputFragment);

impl RustOutputFragment {
    pub fn kind(&self) -> ffi::OutputKind {
        match &self.inner {
            OutputFragment::Effect(_) => ffi::OutputKind::Effect,
            OutputFragment::Hr => ffi::OutputKind::Hr,
            OutputFragment::LineBreak => ffi::OutputKind::LineBreak,
            OutputFragment::MxpError(_) => ffi::OutputKind::MxpError,
            OutputFragment::MxpEntity(EntityFragment::Set { .. }) => ffi::OutputKind::MxpEntitySet,
            OutputFragment::MxpEntity(_) => ffi::OutputKind::MxpEntityUnset,
            OutputFragment::PageBreak => ffi::OutputKind::PageBreak,
            OutputFragment::Telnet(_) => ffi::OutputKind::Telnet,
            OutputFragment::Text(_) => ffi::OutputKind::Text,
            OutputFragment::Frame(_) => unimplemented!("<frame>"),
            OutputFragment::Image(_) => unimplemented!("<image>"),
        }
    }

    pub fn effect(&self) -> &RustEffectFragment {
        match &self.inner {
            OutputFragment::Effect(fragment) => fragment.into(),
            _ => panic!("expected Effect, found {:?}", self.kind()),
        }
    }

    pub fn mxp_entity_set(&self) -> ffi::MxpEntitySet {
        match &self.inner {
            OutputFragment::MxpEntity(EntityFragment::Set {
                name,
                value,
                publish,
                is_variable,
            }) => ffi::MxpEntitySet {
                name: name.into(),
                value: value.into(),
                publish: *publish,
                is_variable: *is_variable,
            },
            _ => panic!("expected MxpEntitySet, found {:?}", self.kind()),
        }
    }

    pub fn mxp_entity_unset(&self) -> ffi::MxpEntityUnset {
        match &self.inner {
            OutputFragment::MxpEntity(EntityFragment::Unset { name, is_variable }) => {
                ffi::MxpEntityUnset {
                    name: name.into(),
                    is_variable: *is_variable,
                }
            }
            _ => panic!("expected MxpEntityUnset, found {:?}", self.kind()),
        }
    }

    pub fn mxp_error(&self) -> QString {
        match &self.inner {
            OutputFragment::MxpError(error) => QString::from(&error.to_string()),
            _ => panic!("expected MxpError, found {:?}", self.kind()),
        }
    }

    pub fn telnet(&self) -> &RustTelnetFragment {
        match &self.inner {
            OutputFragment::Telnet(request) => request.into(),
            _ => panic!("expected Telnet, found {:?}", self.kind()),
        }
    }

    pub fn text(&self) -> &RustTextFragment {
        match &self.inner {
            OutputFragment::Text(text) => text.into(),
            _ => panic!("expected Text, found {:?}", self.kind()),
        }
    }
}
