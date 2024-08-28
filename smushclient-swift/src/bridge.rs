use enumeration::EnumSet;
use mud_stream::nonblocking::MudStream;
use mud_transformer::Tag;
use std::fs::File;
use std::path::Path;
use std::{mem, vec};
use tokio::io::AsyncWriteExt;
use tokio::net::TcpStream;

use crate::client::{AliasHandler, ClientHandler};
use crate::error::StringifyResultError;
use crate::ffi;
use crate::sync::NonBlockingMutex;
use smushclient::{AliasOutcome, SmushClient, World};

const SUPPORTED_TAGS: EnumSet<Tag> = enums![
    Tag::Bold,
    Tag::Color,
    Tag::Font,
    Tag::H1,
    Tag::H2,
    Tag::H3,
    Tag::H4,
    Tag::H5,
    Tag::H6,
    Tag::Highlight,
    Tag::Hr,
    Tag::Hyperlink,
    Tag::Italic,
    Tag::Send,
    Tag::Strikeout,
    Tag::Underline
];

pub struct RustAliasOutcome {
    outcome: AliasOutcome,
    requests: Vec<ffi::SendRequest>,
}

impl RustAliasOutcome {
    pub fn new(outcome: AliasOutcome, requests: Vec<ffi::SendRequest>) -> Self {
        Self { outcome, requests }
    }

    pub fn stream(&mut self) -> RustSendStream {
        RustSendStream {
            inner: mem::take(&mut self.requests).into_iter(),
        }
    }

    pub fn should_display(&self) -> bool {
        self.outcome.display
    }

    pub fn should_remember(&self) -> bool {
        self.outcome.remember
    }

    pub fn should_send(&self) -> bool {
        self.outcome.send
    }
}

#[repr(transparent)]
pub struct RustSendStream {
    inner: vec::IntoIter<ffi::SendRequest>,
}

impl RustSendStream {
    #[inline(always)]
    pub fn next(&mut self) -> Option<ffi::SendRequest> {
        self.inner.next()
    }

    #[inline(always)]
    pub fn count(&self) -> usize {
        self.inner.len()
    }
}

#[repr(transparent)]
pub struct RustOutputStream {
    inner: vec::IntoIter<ffi::OutputFragment>,
}

impl RustOutputStream {
    #[inline(always)]
    pub fn next(&mut self) -> Option<ffi::OutputFragment> {
        self.inner.next()
    }

    #[inline(always)]
    pub fn count(&self) -> usize {
        self.inner.len()
    }
}

#[derive(Default)]
pub struct RustMudBridge {
    stream: Option<MudStream<TcpStream>>,
    client: SmushClient,
    input_lock: NonBlockingMutex,
    output_lock: NonBlockingMutex,
    stream_lock: NonBlockingMutex,
}

impl RustMudBridge {
    pub fn new(world: World) -> Self {
        Self {
            client: SmushClient::new(world, SUPPORTED_TAGS),
            ..Default::default()
        }
    }

    pub fn load<P: AsRef<Path>>(path: P) -> Result<Self, String> {
        let file = File::open(path).str()?;
        World::load(file).map(Self::new).str()
    }

    pub fn save<P: AsRef<Path>>(&self, path: P) -> Result<(), String> {
        let file = File::create(path).str()?;
        self.client.world().save(file).str()
    }

    pub fn world(&self) -> World {
        self.client.world().clone()
    }

    pub fn set_world(&mut self, world: World) {
        let config = self.client.set_world(world);
        if let Some(ref mut stream) = &mut self.stream {
            stream.set_config(config);
        }
    }

    pub fn connected(&self) -> bool {
        self.stream.is_some()
    }

    #[allow(clippy::needless_pass_by_value)]
    pub fn alias(&mut self, command: String) -> RustAliasOutcome {
        let mut handler = AliasHandler::new();
        let lock = self.output_lock.lock();
        let outcome = self.client.alias(&command, &mut handler);
        drop(lock);
        RustAliasOutcome::new(outcome, handler.into())
    }

    pub async fn connect(&mut self) -> Result<(), String> {
        let world = self.client.world();
        let stream = TcpStream::connect((world.site.clone(), world.port))
            .await
            .str()?;
        let locks = (
            self.stream_lock.lock(),
            self.output_lock.lock(),
            self.input_lock.lock(),
        );
        self.stream = Some(MudStream::new(stream, self.client.config()));
        drop(locks);
        Ok(())
    }

    pub async fn disconnect(&mut self) -> Result<(), String> {
        let locks = (
            self.stream_lock.lock(),
            self.output_lock.lock(),
            self.input_lock.lock(),
        );
        let result = match self.stream {
            Some(ref mut stream) => stream.shutdown().await.str(),
            None => Ok(()),
        };
        drop(locks);
        result
    }

    pub async fn receive(&mut self) -> Result<RustOutputStream, String> {
        let lock = self.stream_lock.lock();
        let result = match self.stream {
            Some(ref mut stream) => stream.read().await.str()?,
            None => None,
        };
        drop(lock);
        let lock = self.output_lock.lock();
        let mut handler = ClientHandler::new();
        if let Some(output) = result {
            self.client.receive(output, &mut handler);
        }
        drop(lock);
        Ok(RustOutputStream {
            inner: handler.into_iter(),
        })
    }

    pub async fn send(&mut self, input: String) -> Result<(), String> {
        let input = input.as_bytes();
        let lock = self.input_lock.lock();
        let result = match self.stream {
            Some(ref mut stream) => stream.write_all(input).await.str(),
            None => Ok(()),
        };
        drop(lock);
        result
    }
}
