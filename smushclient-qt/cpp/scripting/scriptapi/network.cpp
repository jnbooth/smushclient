#include "../../ui/worldtab.h"
#include "../scriptapi.h"
#include <QtNetwork/QHostInfo>

// Public static methods

QList<QHostAddress>
ScriptApi::GetHostAddress(const QString& hostName)
{
  return QHostInfo::fromName(hostName).addresses();
}

QString
ScriptApi::GetHostName(const QString& address)
{
  return QHostInfo::fromName(address).hostName();
}

// Public methods

ApiCode
ScriptApi::Connect() const
{
  return tab.connectToHost() ? ApiCode::OK : ApiCode::WorldOpen;
}

ApiCode
ScriptApi::Disconnect() const
{
  return tab.connectToHost() ? ApiCode::OK : ApiCode::WorldClosed;
}

int64_t
ScriptApi::GetReceivedBytes() const noexcept
{
  return static_cast<int64_t>(client.bytesReceived());
}

int64_t
ScriptApi::GetSentBytes() const noexcept
{
  return totalBytesSent;
}

bool
ScriptApi::IsConnected() const
{
  return socket.isOpen();
}
