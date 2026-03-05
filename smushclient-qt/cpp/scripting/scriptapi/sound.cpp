#include "../../client.h"
#include "../callback/plugincallback.h"
#include "../scriptapi.h"

using std::string_view;

// Public methods

ApiCode
ScriptApi::PlaySound(size_t channel, string_view path, bool loop, float volume)
{
  OnPluginPlaySound onPlaySound(path);
  sendCallback(onPlaySound);
  if (onPlaySound.discarded()) {
    return ApiCode::OK;
  }
  return client.playFile(channel, path, volume, loop);
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
ScriptApi::StopSound(size_t channel)
{
  OnPluginPlaySound onPlaySound("");
  sendCallback(onPlaySound);
  if (onPlaySound.discarded()) {
    return ApiCode::OK;
  }
  return client.stopSound(channel);
}

AudioSinkStatus
ScriptApi::GetSoundStatus(size_t channel) const noexcept
{
  return client.soundStatus(channel);
}
