#include "audio.h"
#include <QtCore/QFile>

AudioChannel::AudioChannel()
    : buffer(),
      output(),
      player()
{
  player.setAudioOutput(&output);
}

void AudioChannel::playBuffer(const QByteArray &data, bool loop, float volume)
{
  player.stop();
  output.setVolume(volume);
  buffer.close();
  buffer.setData(data);
  buffer.open(QIODevice::ReadOnly | QIODevice::Unbuffered);
  buffer.seek(0);
  player.setSourceDevice(&buffer);
  player.setLoops(loop ? QMediaPlayer::Infinite : 0);
  player.play();
}

void AudioChannel::playFile(const QUrl &url, bool loop, float volume)
{
  player.stop();
  player.setSource(url);
  player.setLoops(loop ? QMediaPlayer::Infinite : 0);
  output.setVolume(volume);
  player.play();
}
