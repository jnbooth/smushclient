#include "connecting.h"
#include "ui_connecting.h"

PrefsConnecting::PrefsConnecting(World *world, QWidget *parent)
    : AbstractPrefsPane(parent), ui(new Ui::PrefsConnecting), world(world)
{
  ui->setupUi(this);
  CONNECT_WORLD(Player);
  CONNECT_WORLD(Password);
  CONNECT_WORLD(ConnectMethod);

  ui->ConnectText->setPlainText(world->getConnectText());
}

PrefsConnecting::~PrefsConnecting()
{
  delete ui;
}

void PrefsConnecting::on_ConnectText_textChanged()
{
  world->setConnectText(ui->ConnectText->toPlainText());
}
