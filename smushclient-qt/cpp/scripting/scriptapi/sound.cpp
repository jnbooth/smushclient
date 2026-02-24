#include "../../client.h"
#include "../scriptapi.h"

using std::string_view;

// Public methods

ApiCode
ScriptApi::PlaySound(size_t channel,
                     string_view path,
                     bool loop,
                     float volume) const noexcept
{
  return client.playFile(channel, path, volume, loop);
}

ApiCode
ScriptApi::PlaySound(size_t channel,
                     const QString& path,
                     bool loop,
                     float volume) const noexcept
{
  const QByteArray utf8 = path.toUtf8();
  return PlaySound(
    channel, string_view(utf8.data(), utf8.size()), loop, volume);
}

ApiCode
ScriptApi::PlaySoundMemory(size_t channel,
                           QByteArrayView sound,
                           bool loop,
                           float volume) const noexcept
{
  return client.playBuffer(channel, sound, volume, loop);
}

ApiCode
ScriptApi::StopSound(size_t channel) const noexcept
{
  return client.stopSound(channel);
}
