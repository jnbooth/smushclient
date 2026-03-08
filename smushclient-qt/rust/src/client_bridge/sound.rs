use cxx_qt::CxxQtType;

use crate::ffi::{self, ApiCode, BytesView, StringView};
use crate::results::IntoApiCode;

impl ffi::SmushClient {
    pub fn handle_alert(&self) -> ApiCode {
        let client = &self.rust().client;
        let sound = &client.borrow_world().new_activity_sound;
        if sound.is_empty() {
            return ffi::ApiCode::OK;
        }
        client.play_file_raw(sound).code()
    }

    pub fn play_buffer(&self, i: usize, buf: BytesView<'_>, volume: f32, looping: bool) -> ApiCode {
        self.client
            .play_buffer(i, buf.to_vec(), volume, looping.into())
            .code()
    }

    pub fn play_file(&self, i: usize, path: StringView<'_>, volume: f32, looping: bool) -> ApiCode {
        let client = &self.rust().client;
        if path.is_empty() {
            return client
                .configure_audio_sink(i, volume, looping.into())
                .code();
        }
        let Ok(path) = path.to_str() else {
            return ApiCode::FileNotFound;
        };
        client.play_file(i, path, volume, looping.into()).code()
    }

    pub fn play_file_raw(&self, path: StringView<'_>) -> ApiCode {
        let Ok(path) = path.to_str() else {
            return ApiCode::FileNotFound;
        };
        self.rust().client.play_file_raw(path).code()
    }

    pub fn sound_status(&self, i: usize) -> ffi::AudioSinkStatus {
        self.rust().client.sound_status(i).into()
    }

    pub fn stop_sound(&self, i: usize) -> ApiCode {
        self.rust().client.stop_sound(i).code()
    }
}
