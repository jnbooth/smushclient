use sha1::{Digest, Sha1};
use uuid::Uuid;

pub fn create_guid(buf: &mut [u8]) -> &str {
    Uuid::new_v4().hyphenated().encode_upper(buf)
}

pub fn get_unique_id(buf: &mut [u8]) -> &str {
    let mut guid = [0u8; uuid::fmt::Simple::LENGTH];
    Uuid::new_v4().simple().encode_upper(&mut guid);
    let digest = sha1(&guid);
    encode_hex(&digest[..12], buf)
}

pub fn hash<'a>(data: &[u8], buf: &'a mut [u8]) -> &'a str {
    let digest = sha1(data);
    encode_hex(&digest, buf)
}

fn encode_hex<'a>(input: &[u8], output: &'a mut [u8]) -> &'a str {
    const DIGITS: &[u8; 16] = b"0123456789abcdef";
    let output = &mut output[..input.len() * 2];
    let mut out = output.iter_mut();
    for &byte in input {
        *out.next().unwrap() = DIGITS[(byte >> 4) as usize];
        *out.next().unwrap() = DIGITS[(byte & 0x0F) as usize];
    }
    // SAFETY: Span is valid UTF-8, containing only 0-9 and A-F.
    unsafe { str::from_utf8_unchecked(output) }
}

fn sha1(data: &[u8]) -> sha1::digest::Output<Sha1> {
    let mut hasher = Sha1::new();
    hasher.update(data);
    hasher.finalize()
}
