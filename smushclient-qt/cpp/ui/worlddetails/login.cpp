#include "login.h"
#include "ui_login.h"
#include "../../fieldconnector.h"

PrefsLogin::PrefsLogin(World &world, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::PrefsLogin),
      empty(true),
      single(true),
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
  const QString text = ui->ConnectText->toPlainText();
  empty = text.isEmpty();
  world.setConnectText(text);
  if (single) {
    ui->ConnectText_lines->setNum(empty ? 0 : 1);
  }
}

void PrefsLogin::on_ConnectText_blockCountChanged(int count)
{
  single = count <= 1;
  ui->ConnectText_lines->setNum(count == 1 && empty ? 0 : count);
}
