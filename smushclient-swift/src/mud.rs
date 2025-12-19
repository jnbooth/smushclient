use std::fs::File;
use std::path::Path;

use mud_transformer::Tag;
use smushclient::{CommandSource, SmushClient, World};
use tokio::io::AsyncWriteExt;
use tokio::net::TcpStream;

use crate::client::{AliasHandler, ClientHandler};
use crate::error::StringifyResultError;
use crate::stream::{RustAliasOutcome, RustOutputStream};

const BUF_LEN: usize = 1024 * 20;

#[derive(Default)]
pub struct RustMudBridge {
    stream: Option<TcpStream>,
    read_buf: Vec<u8>,
    write_buf: Vec<u8>,
    client: SmushClient,
}

impl RustMudBridge {
    pub fn new(world: World) -> Self {
        Self {
            read_buf: vec![0; BUF_LEN],
            write_buf: Vec::new(),
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

    pub fn set_world(&mut self, world: World) -> Result<bool, String> {
        self.client.update_world(world).str()
    }

    pub fn connected(&self) -> bool {
        self.stream.is_some()
    }

    #[allow(clippy::needless_pass_by_value)]
    pub fn alias(&mut self, command: String) -> RustAliasOutcome {
        let mut handler = AliasHandler::new();
        let outcome = self
            .client
            .alias(&command, CommandSource::User, &mut handler);
        RustAliasOutcome::new(outcome, handler.into())
    }

    pub async fn connect(&mut self) -> Result<(), String> {
        let world = self.client.world();
        let stream = TcpStream::connect((world.site.clone(), world.port))
            .await
            .str()?;
        self.stream = Some(stream);
        Ok(())
    }

    pub async fn disconnect(&mut self) -> Result<(), String> {
        match self.stream {
            Some(ref mut stream) => stream.shutdown().await.str(),
            None => Ok(()),
        }
    }

    pub async fn receive(&mut self) -> Result<RustOutputStream, String> {
        let Some(stream) = &mut self.stream else {
            return Ok(RustOutputStream::new(Vec::new().into_iter()));
        };
        let read_result = self.client.read_async(stream, &mut self.read_buf).await;

        let mut handler = ClientHandler::new();
        self.client.flush_output(&mut handler);

        if let Err(e) = read_result {
            let output = handler.into_iter();
            if output.len() == 0 {
                return Err(e.to_string());
            }
            return Ok(RustOutputStream::new(output));
        }
        self.write_buf.clear();
        self.client
            .write(&mut self.write_buf)
            .map_err(|e| e.to_string())?;
        stream
            .write_all(&self.write_buf)
            .await
            .map_err(|e| e.to_string())?;
        Ok(RustOutputStream::new(handler.into_iter()))
    }

    pub async fn send(&mut self, input: String) -> Result<(), String> {
        let input = input.as_bytes();
        match self.stream {
            Some(ref mut stream) => stream.write_all(input).await.str(),
            None => Ok(()),
        }
    }
}
