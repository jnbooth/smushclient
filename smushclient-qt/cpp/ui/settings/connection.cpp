#include "connection.h"
#include "ui_connection.h"
#include "../../fieldconnector.h"
#include "../../settings.h"

SettingsConnecting::SettingsConnecting(Settings &settings, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::SettingsConnecting)
{
  ui->setupUi(this);
  CONNECT_SETTINGS(AutoConnect);
  CONNECT_SETTINGS(ReconnectOnDisconnect);
  CONNECT_SETTINGS(DisplayConnect);
  CONNECT_SETTINGS(DisplayDisconnect);
}

SettingsConnecting::~SettingsConnecting()
{
  delete ui;
}
