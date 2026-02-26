#include "../../ui/ui_worldtab.h"
#include "../../ui/worldtab.h"
#include "../scriptapi.h"

using std::string_view;

// Public methods

ApiCode
ScriptApi::Execute(const QString& command) const noexcept
{
  if (!tab.isConnected()) {
    return ApiCode::WorldClosed;
  }
  tab.sendCommand(command, CommandSource::Execute);
  return ApiCode::OK;
}

ApiCode
ScriptApi::LogSend(QByteArray& bytes)
{
  return sendToWorld(bytes, SendFlag::Echo | SendFlag::Log);
}

ApiCode
ScriptApi::Send(string_view text)
{
  QByteArray bytes(text);
  return sendToWorld(bytes, SendFlag::Echo);
}

ApiCode
ScriptApi::Send(const QString& text)
{
  return sendToWorld(text, SendFlag::Echo);
}

ApiCode
ScriptApi::Send(QByteArray& bytes)
{
  return sendToWorld(bytes, SendFlag::Echo);
}

ApiCode
ScriptApi::SendImmediate(QByteArray& bytes)
{
  return sendToWorld(bytes, SendFlag::Echo | SendFlag::Immediate);
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

ApiCode
ScriptApi::SendPush(QByteArray& bytes)
{
  const QString command = QString::fromUtf8(bytes);
  tab.ui->input->remember(command);
  return sendToWorld(bytes, command, SendFlag::Echo);
}
