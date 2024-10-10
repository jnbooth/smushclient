#pragma once
#include <QtCore/QBuffer>
#include <QtMultimedia/QAudioFormat>
#include <QtMultimedia/QAudioSink>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QAudioOutput>

class AudioChannel
{
public:
  AudioChannel();

  inline bool isPlaying() const { return player.isPlaying(); }
  inline void pause() { player.pause(); }
  inline void play() { player.play(); }
  void playBuffer(const QByteArray &data, bool loop = false, float volume = 1.0);
  void playFile(const QUrl &url, bool loop = false, float volume = 1.0);
  inline void playFile(const QString &path, bool loop = false, float volume = 1.0)
  {
    playFile(QUrl::fromLocalFile(path), loop, volume);
  }
  inline void stop() { player.stop(); }

private:
  QBuffer buffer;
  QAudioOutput output;
  QMediaPlayer player;
};
