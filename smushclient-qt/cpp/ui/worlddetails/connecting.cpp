#include "connecting.h"
#include "../../fieldconnector.h"
#include "smushclient_qt/src/ffi/world.cxxqt.h"
#include "ui_connecting.h"

#ifndef QT_NO_SSL
#include <QtNetwork/QSslSocket>
#endif

PrefsConnecting::PrefsConnecting(const World& world, QWidget* parent)
  : QWidget(parent)
  , ui(new Ui::PrefsConnecting)
{
  ui->setupUi(this);
  CONNECT_WORLD(Name);
  CONNECT_WORLD(Site);
  CONNECT_WORLD(Port);
  CONNECT_WORLD(UseSsl);
  CONNECT_WORLD(UseProxy);
  CONNECT_WORLD(ProxyServer);
  CONNECT_WORLD(ProxyPort);
  CONNECT_WORLD(ProxyUsername);
  CONNECT_WORLD(ProxyPassword);
  CONNECT_WORLD(SaveWorldAutomatically);

#ifndef QT_NO_SSL
  if (!QSslSocket::supportsSsl())
#endif
    ui->UseSsl->setDisabled(true);
}

PrefsConnecting::~PrefsConnecting()
{
  delete ui;
}
