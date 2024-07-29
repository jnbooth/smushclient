use crate::handler::Handler;
use crate::plugins::PluginEngine;
use mud_transformer::{EffectFragment, OutputDrain, OutputFragment};
use smushclient_plugins::Plugin;

pub struct SmushClient {
    output_buf: Vec<OutputFragment>,
    line_text: String,
    plugins: PluginEngine,
}

impl SmushClient {
    pub const fn new() -> Self {
        Self {
            output_buf: Vec::new(),
            line_text: String::new(),
            plugins: PluginEngine::new(),
        }
    }

    pub fn receive<H: Handler>(&mut self, mut output: OutputDrain, handler: &mut H) {
        if self.output_buf.is_empty() {
            receive_lines(&mut output, handler, &mut self.plugins, &mut self.line_text);
            self.output_buf.extend(output);
            return;
        }
        self.output_buf.extend(output);
        let last_break = match self.output_buf.iter().rposition(is_break) {
            Some(last_break) => last_break,
            None => return,
        };
        let mut output = self.output_buf.drain(..=last_break);
        receive_lines(&mut output, handler, &mut self.plugins, &mut self.line_text);
    }

    pub fn alias<H: Handler>(&mut self, input: &str, handler: &mut H) -> bool {
        self.plugins.alias(input, handler).suppress
    }

    pub fn load_plugins<I: IntoIterator<Item = Plugin>>(&mut self, iter: I) {
        self.plugins.load_plugins(iter);
    }
}

fn is_nonvisual_output(fragment: &OutputFragment) -> bool {
    matches!(
        fragment,
        OutputFragment::Effect(EffectFragment::Beep) | OutputFragment::Telnet(_)
    )
}

fn is_break(fragment: &OutputFragment) -> bool {
    matches!(
        fragment,
        OutputFragment::Hr | OutputFragment::LineBreak | OutputFragment::PageBreak
    )
}

fn receive_lines<H: Handler>(
    output: &mut OutputDrain,
    handler: &mut H,
    plugins: &mut PluginEngine,
    line_text: &mut String,
) {
    loop {
        let slice = output.as_slice();
        if slice.is_empty() {
            return;
        }
        let mut until = 0;
        for (i, fragment) in slice.iter().enumerate() {
            match fragment {
                OutputFragment::Text(fragment) => line_text.push_str(&fragment.text),
                OutputFragment::Hr | OutputFragment::LineBreak | OutputFragment::PageBreak => {
                    until = i + 1;
                    break;
                }
                _ => (),
            }
        }
        if until == 0 {
            return;
        }
        let trigger_effects = plugins.trigger(line_text, handler);
        if trigger_effects.suppress {
            for _ in 0..until {
                let fragment = output.next().unwrap();
                if is_nonvisual_output(&fragment) {
                    handler.display(fragment);
                }
            }
        } else {
            for _ in 0..until {
                handler.display(output.next().unwrap());
            }
        }
    }
}
