#include "../scriptapi.h"

using std::string_view;

// Public methods
ApiCode
ScriptApi::Send(string_view text)
{
  QByteArray bytes(text);
  return sendToWorld(bytes, SendFlag::Echo);
}
ApiCode
ScriptApi::Send(const QString& text)
{
  QByteArray bytes = text.toUtf8();
  return sendToWorld(bytes, SendFlag::Echo);
}

ApiCode
ScriptApi::Send(QByteArray& bytes)
{
  return sendToWorld(bytes, SendFlag::Echo);
}

ApiCode
ScriptApi::SendNoEcho(QByteArray& bytes)
{
  return sendToWorld(bytes, SendFlags());
}
ApiCode
ScriptApi::SendPacket(QByteArrayView bytes)
{
  ++totalPacketsSent;
  if (socket.write(bytes.data(), bytes.size()) == -1) [[unlikely]] {
    return ApiCode::WorldClosed;
  }

  return ApiCode::OK;
}
