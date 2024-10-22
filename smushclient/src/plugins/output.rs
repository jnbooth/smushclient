use std::mem;

use mud_transformer::{EffectFragment, OutputFragment, TextFragment, TextStyle};

use crate::World;

pub fn is_nonvisual_output(fragment: &OutputFragment) -> bool {
    matches!(
        fragment,
        OutputFragment::Effect(EffectFragment::Beep) | OutputFragment::Telnet(_)
    )
}

pub fn alter_text_output(fragment: &mut TextFragment, world: &World) {
    if fragment.flags.contains(TextStyle::Inverse) {
        mem::swap(&mut fragment.foreground, &mut fragment.background);
    }
    if !world.show_bold {
        fragment.flags.remove(TextStyle::Bold);
    }
    if !world.show_italic {
        fragment.flags.remove(TextStyle::Italic);
    }
    if !world.show_underline {
        fragment.flags.remove(TextStyle::Underline);
    }
    if fragment.action.is_none() {
        return;
    }
    if world.underline_hyperlinks {
        fragment.flags.insert(TextStyle::Underline);
    }
    if world.use_custom_link_colour {
        fragment.foreground = world.hyperlink_colour;
    }
}
