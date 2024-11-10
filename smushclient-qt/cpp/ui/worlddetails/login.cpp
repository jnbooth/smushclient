#include "login.h"
#include "ui_login.h"
#include "../../fieldconnector.h"

PrefsLogin::PrefsLogin(World &world, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::PrefsLogin),
      world(world)
{
  ui->setupUi(this);
  CONNECT_WORLD(Player);
  CONNECT_WORLD(Password);
  CONNECT_WORLD(ConnectMethod);

  ui->ConnectText->setPlainText(world.getConnectText());
}

PrefsLogin::~PrefsLogin()
{
  delete ui;
}

// Private methods

void PrefsLogin::on_ConnectText_textChanged()
{
  world.setConnectText(ui->ConnectText->toPlainText());
}
