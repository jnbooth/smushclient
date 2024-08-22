#include "address.h"
#include "ui_address.h"

PrefsAddress::PrefsAddress(const World *world, QWidget *parent)
    : AbstractPrefsPane(parent), ui(new Ui::PrefsAddress)
{
  ui->setupUi(this);
  CONNECT_WORLD(Name);
  CONNECT_WORLD(Site);
  CONNECT_WORLD(Port);
  CONNECT_WORLD(ProxyType);
  CONNECT_WORLD(ProxyServer);
  CONNECT_WORLD(ProxyPort);
  CONNECT_WORLD(SaveWorldAutomatically);
}

PrefsAddress::~PrefsAddress()
{
  delete ui;
}
