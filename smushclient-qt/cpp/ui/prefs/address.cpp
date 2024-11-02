#include "address.h"
#include "ui_address.h"
#include "../../fieldconnector.h"

PrefsAddress::PrefsAddress(const World &world, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::PrefsAddress)
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

PrefsAddress::~PrefsAddress()
{
  delete ui;
}
