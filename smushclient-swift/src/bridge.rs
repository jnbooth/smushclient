use mud_stream::nonblocking::MudStream;
use std::vec;
use tokio::io::AsyncWriteExt;
use tokio::net::TcpStream;

use crate::client::ClientHandler;
use crate::sync::SimpleLock;
use crate::FfiOutputFragment;
use smushclient::SmushClient;

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
    address: String,
    port: u16,
    stream: Option<MudStream<TcpStream>>,
    client: SmushClient,
    input_lock: SimpleLock,
    output_lock: SimpleLock,
}

impl RustMudBridge {
    pub fn new(address: String, port: u16) -> Self {
        Self {
            address,
            port,
            ..Default::default()
        }
    }

    pub fn connected(&self) -> bool {
        self.stream.is_some()
    }

    pub async fn connect(&mut self) -> Result<(), String> {
        let stream = TcpStream::connect((self.address.clone(), self.port))
            .await
            .map_err(|e| e.to_string())?;
        let locks = (self.output_lock.lock(), self.input_lock.lock());
        self.stream = Some(MudStream::new(stream, Default::default()));
        drop(locks);
        Ok(())
    }

    pub async fn disconnect(&mut self) -> Result<(), String> {
        let locks = (self.output_lock.lock(), self.input_lock.lock());
        let result = match self.stream {
            Some(ref mut stream) => stream.shutdown().await.map_err(|e| e.to_string()),
            None => Ok(()),
        };
        drop(locks);
        result
    }

    pub async fn receive(&mut self) -> Result<RustOutputStream, String> {
        let lock = self.output_lock.lock();
        let result = match self.stream {
            Some(ref mut stream) => stream.read().await.map_err(|e| e.to_string())?,
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
            Some(ref mut stream) => stream.write_all(input).await.map_err(|e| e.to_string()),
            None => Ok(()),
        };
        drop(lock);
        result
    }
}
