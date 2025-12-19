use std::fs::File;
use std::path::Path;

use mud_transformer::Tag;
use smushclient::{CommandSource, SmushClient, World};
use tokio::io::AsyncWriteExt;
use tokio::net::TcpStream;

use crate::client::{AliasHandler, ClientHandler};
use crate::error::StringifyResultError;
use crate::stream::{RustAliasOutcome, RustOutputStream};
use crate::sync::NonBlockingMutex;

const BUF_LEN: usize = 1024 * 20;

#[derive(Default)]
pub struct RustMudBridge {
    stream: Option<TcpStream>,
    read_buf: Vec<u8>,
    client: SmushClient,
    input_lock: NonBlockingMutex,
    output_lock: NonBlockingMutex,
    stream_lock: NonBlockingMutex,
}

impl RustMudBridge {
    pub fn new(world: World) -> Self {
        Self {
            read_buf: vec![0; BUF_LEN],
            client: SmushClient::new(
                world,
                Tag::Bold
                    | Tag::Color
                    | Tag::Font
                    | Tag::H1
                    | Tag::H2
                    | Tag::H3
                    | Tag::H4
                    | Tag::H5
                    | Tag::H6
                    | Tag::Highlight
                    | Tag::Hr
                    | Tag::Hyperlink
                    | Tag::Italic
                    | Tag::Send
                    | Tag::Strikeout
                    | Tag::Underline,
            ),
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
        self.client.update_world(world);
    }

    pub fn connected(&self) -> bool {
        self.stream.is_some()
    }

    #[allow(clippy::needless_pass_by_value)]
    pub fn alias(&mut self, command: String) -> RustAliasOutcome {
        let mut handler = AliasHandler::new();
        let lock = self.output_lock.lock();
        let outcome = self
            .client
            .alias(&command, CommandSource::User, &mut handler);
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
        self.stream = Some(stream);
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
        let Some(stream) = &mut self.stream else {
            return Ok(RustOutputStream::new(Vec::new().into_iter()));
        };
        let lock = self.output_lock.lock();
        let read_result = self.client.read_async(stream, &mut self.read_buf).await;

        let mut handler = ClientHandler::new();
        self.client.flush_output(&mut handler);
        drop(lock);

        if let Err(e) = read_result {
            let output = handler.into_iter();
            if output.len() == 0 {
                return Err(e.to_string());
            }
            return Ok(RustOutputStream::new(output));
        }
        let lock = self.input_lock.lock();
        if let Err(e) = self.client.write_async(stream).await {
            return Err(e.to_string());
        }
        drop(lock);
        Ok(RustOutputStream::new(handler.into_iter()))
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
