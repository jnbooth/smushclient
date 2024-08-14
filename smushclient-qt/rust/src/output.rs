use super::ffi;
use cxx_qt_lib::{QByteArray, QColor, QList, QString, QStringList};
use mud_transformer::mxp::{self, RgbColor, SendTo};
use mud_transformer::{
    EffectFragment, Output, OutputFragment, TelnetFragment, TextFragment, TextStyle,
};

fn encode_color(color: RgbColor) -> QColor {
    QColor::from_rgb(color.r as i32, color.g as i32, color.b as i32)
}

impl From<&mxp::Link> for ffi::MxpLink {
    fn from(action: &mxp::Link) -> Self {
        let hint = match action.hint {
            Some(ref hint) => hint.as_str().into(),
            None => QString::default(),
        };
        let mut prompts = QList::default();
        for prompt in &action.prompts {
            prompts.append(QString::from(prompt));
        }
        Self {
            action: action.action.as_str().into(),
            hint,
            prompts: QStringList::from(&prompts),
            sendto: action.sendto.into(),
        }
    }
}

#[repr(transparent)]
pub struct RustTextFragment {
    inner: TextFragment,
}

impl From<TextFragment> for RustTextFragment {
    fn from(inner: TextFragment) -> Self {
        Self { inner }
    }
}

macro_rules! flag_method {
    ($n:ident, $v:expr) => {
        #[inline]
        pub fn $n(&self) -> bool {
            self.inner.flags.contains($v)
        }
    };
}

impl RustTextFragment {
    #[inline]
    pub fn text(&self) -> QString {
        QString::from(&*self.inner.text)
    }

    #[inline]
    pub fn foreground(&self) -> QColor {
        encode_color(self.inner.foreground)
    }

    #[inline]
    pub fn background(&self) -> QColor {
        encode_color(self.inner.background)
    }

    #[inline]
    pub fn has_link(&self) -> bool {
        self.inner.action.is_some()
    }

    #[inline]
    pub fn link(&self) -> ffi::MxpLink {
        self.inner.action.as_ref().unwrap().into()
    }

    flag_method!(is_blink, TextStyle::Blink);
    flag_method!(is_bold, TextStyle::Bold);
    flag_method!(is_highlight, TextStyle::Highlight);
    flag_method!(is_inverse, TextStyle::Inverse);
    flag_method!(is_italic, TextStyle::Italic);
    flag_method!(is_strikeout, TextStyle::Strikeout);
    flag_method!(is_underline, TextStyle::Underline);
}

impl_convert_enum!(ffi::SendTo, SendTo, World, Input, Internet);

impl_convert_enum!(
    ffi::EffectFragment,
    EffectFragment,
    Backspace,
    Beep,
    CarriageReturn,
    EraseCharacter,
    EraseLine
);

#[repr(transparent)]
pub struct RustTelnetFragment {
    inner: TelnetFragment,
}

impl From<TelnetFragment> for RustTelnetFragment {
    fn from(inner: TelnetFragment) -> Self {
        Self { inner }
    }
}

impl RustTelnetFragment {
    pub fn kind(&self) -> ffi::TelnetRequest {
        match self.inner {
            TelnetFragment::Afk { .. } => ffi::TelnetRequest::Afk,
            TelnetFragment::Do { .. } => ffi::TelnetRequest::Do,
            TelnetFragment::IacGa => ffi::TelnetRequest::IacGa,
            TelnetFragment::Naws => ffi::TelnetRequest::Naws,
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
            TelnetFragment::Afk { challenge } => QByteArray::from(&**challenge),
            TelnetFragment::Subnegotiation { data, .. } => QByteArray::from(&**data),
            _ => QByteArray::default(),
        }
    }
}

#[repr(transparent)]
pub struct OutputFragmentRust {
    inner: OutputFragment,
}

impl From<OutputFragment> for OutputFragmentRust {
    fn from(inner: OutputFragment) -> Self {
        Self { inner }
    }
}

pub enum RustOutputFragment {
    Effect(ffi::EffectFragment),
    Hr,
    Image(String),
    LineBreak,
    MxpError(String),
    MxpVariable { name: String, value: Option<String> },
    PageBreak,
    Telnet(RustTelnetFragment),
    Text(RustTextFragment),
}

impl From<OutputFragment> for RustOutputFragment {
    fn from(value: OutputFragment) -> Self {
        match value {
            OutputFragment::Effect(effect) => Self::Effect(effect.into()),
            OutputFragment::Hr => Self::Hr,
            OutputFragment::Image(src) => Self::Image(src),
            OutputFragment::LineBreak => Self::LineBreak,
            OutputFragment::MxpError(error) => Self::MxpError(error.to_string()),
            OutputFragment::MxpVariable { name, value } => Self::MxpVariable { name, value },
            OutputFragment::PageBreak => Self::PageBreak,
            OutputFragment::Telnet(telnet) => Self::Telnet(telnet.into()),
            OutputFragment::Text(text) => Self::Text(text.into()),
        }
    }
}

impl From<Output> for RustOutputFragment {
    fn from(value: Output) -> Self {
        Self::from(value.fragment)
    }
}

impl RustOutputFragment {
    pub fn kind(&self) -> ffi::OutputKind {
        match self {
            RustOutputFragment::Effect(_) => ffi::OutputKind::Effect,
            RustOutputFragment::Hr => ffi::OutputKind::Hr,
            RustOutputFragment::Image(_) => ffi::OutputKind::Image,
            RustOutputFragment::LineBreak => ffi::OutputKind::LineBreak,
            RustOutputFragment::MxpError(_) => ffi::OutputKind::MxpError,
            RustOutputFragment::MxpVariable { value: None, .. } => {
                ffi::OutputKind::MxpVariableUnset
            }
            RustOutputFragment::MxpVariable { .. } => ffi::OutputKind::MxpVariableSet,
            RustOutputFragment::PageBreak => ffi::OutputKind::PageBreak,
            RustOutputFragment::Telnet(_) => ffi::OutputKind::Telnet,
            RustOutputFragment::Text(_) => ffi::OutputKind::Text,
        }
    }

    pub fn effect(&self) -> ffi::EffectFragment {
        match self {
            RustOutputFragment::Effect(fragment) => *fragment,
            _ => panic!("expected Effect, found {:?}", self.kind()),
        }
    }

    pub fn image(&self) -> QString {
        match self {
            RustOutputFragment::Image(image) => QString::from(image),
            _ => panic!("expected Image, found {:?}", self.kind()),
        }
    }

    pub fn mxp_error(&self) -> QString {
        match self {
            RustOutputFragment::MxpError(error) => QString::from(error),
            _ => panic!("expected MxpError, found {:?}", self.kind()),
        }
    }

    pub fn mxp_variable_name(&self) -> QString {
        match self {
            RustOutputFragment::MxpVariable { name, .. } => QString::from(name),
            _ => panic!(
                "expected MxpVariableSet or MxpVariableUnset, found {:?}",
                self.kind()
            ),
        }
    }

    pub fn mxp_variable_value(&self) -> QString {
        match self {
            RustOutputFragment::MxpVariable {
                value: Some(value), ..
            } => QString::from(value),
            _ => panic!("expected MxpVariableSet, found {:?}", self.kind()),
        }
    }

    pub fn telnet(&self) -> &RustTelnetFragment {
        match self {
            RustOutputFragment::Telnet(request) => request,
            _ => panic!("expected Telnet, found {:?}", self.kind()),
        }
    }

    pub fn text(&self) -> &RustTextFragment {
        match self {
            RustOutputFragment::Text(text) => text,
            _ => panic!("expected Text, found {:?}", self.kind()),
        }
    }
}
