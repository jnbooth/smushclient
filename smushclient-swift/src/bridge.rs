use mud_stream::nonblocking::MudStream;
use std::fs::File;
use std::path::Path;
use std::vec;
use tokio::io::AsyncWriteExt;
use tokio::net::TcpStream;

use crate::client::ClientHandler;
use crate::error::StringifyResultError;
use crate::sync::NonBlockingMutex;
use crate::FfiOutputFragment;
use smushclient::{SmushClient, World};

#[repr(transparent)]
pub struct RustOutputStream {
    inner: vec::IntoIter<FfiOutputFragment>,
}

impl RustOutputStream {
    #[inline(always)]
    pub fn next(&mut self) -> Option<FfiOutputFragment> {
        self.inner.next()
    }
}

#[derive(Default)]
pub struct RustMudBridge {
    stream: Option<MudStream<TcpStream>>,
    client: SmushClient,
    input_lock: NonBlockingMutex,
    output_lock: NonBlockingMutex,
}

impl RustMudBridge {
    pub fn new(world: World) -> Self {
        Self {
            client: SmushClient::new(world),
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

    pub async fn connect(&mut self) -> Result<(), String> {
        let world = self.client.world();
        let stream = TcpStream::connect((world.site.clone(), world.port))
            .await
            .str()?;
        let locks = (self.output_lock.lock(), self.input_lock.lock());
        self.stream = Some(MudStream::new(stream, self.client.config()));
        drop(locks);
        Ok(())
    }

    pub async fn disconnect(&mut self) -> Result<(), String> {
        let locks = (self.output_lock.lock(), self.input_lock.lock());
        let result = match self.stream {
            Some(ref mut stream) => stream.shutdown().await.str(),
            None => Ok(()),
        };
        drop(locks);
        result
    }

    pub async fn receive(&mut self) -> Result<RustOutputStream, String> {
        let lock = self.output_lock.lock();
        let result = match self.stream {
            Some(ref mut stream) => stream.read().await.str()?,
            None => None,
        };
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
