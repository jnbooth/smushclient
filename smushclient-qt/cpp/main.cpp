#include <QtGui/QGuiApplication>
#include <QtNetwork/QTcpSocket>
#include "cxx-qt-gen/ffi.cxxqt.h"

int main(int argc, char* argv[])
{
  QGuiApplication app(argc, argv);
  QTcpSocket socket;
  socket.connectToHost(QStringLiteral("discworld.atuin.net"), 4242);
  SmushClient client;
  client.read(socket);
  while (client.tryNext()) {
    client.next();
  }
  return app.exec();
}

