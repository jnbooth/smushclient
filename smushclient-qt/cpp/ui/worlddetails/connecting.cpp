#include "connecting.h"
#include "ui_connecting.h"
#include "../../fieldconnector.h"

PrefsConnecting::PrefsConnecting(const World &world, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::PrefsConnecting)
{
  ui->setupUi(this);
  CONNECT_WORLD(Name);
  CONNECT_WORLD(Site);
  CONNECT_WORLD(Port);
  CONNECT_WORLD(UseProxy);
  CONNECT_WORLD(ProxyServer);
  CONNECT_WORLD(ProxyPort);
  CONNECT_WORLD(ProxyUsername);
  CONNECT_WORLD(ProxyPassword);
  CONNECT_WORLD(SaveWorldAutomatically);
}

PrefsConnecting::~PrefsConnecting()
{
  delete ui;
}
