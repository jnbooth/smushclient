#include "worldtab.h"
#include "ui_worldtab.h"
#include <QtWidgets/QDialog>
#include "worldprefs.h"

WorldTab::WorldTab(QWidget *parent)
    : QSplitter(parent), ui(new Ui::WorldTab), client(), world(), socket(this), document(ui->output)
{
  ui->setupUi(this);
  connect(&socket, &QTcpSocket::readyRead, this, &WorldTab::on_socket_ready_read);
}

WorldTab::~WorldTab()
{
  delete ui;
}

const QString &WorldTab::title() const
{
  return world.getName();
}

void WorldTab::openPreferences()
{
  WorldPrefs prefs(&world, this);
  connect(&prefs, &QDialog::finished, this, &WorldTab::on_close_worldprefs);
  prefs.open();
}

bool WorldTab::openWorld(const QString &filename)
{
  if (!client.loadWorld(filename, world))
  {
    return false;
  }
  if (!socket.isOpen())
  {
    socket.connectToHost(world.getSite(), (quint16)world.getPort());
  }
  return true;
}

void WorldTab::on_close_worldprefs(int result)
{
  switch (result)
  {
  case QDialog::Accepted:
    client.setWorld(world);
    break;
  case QDialog::Rejected:
    client.populateWorld(world);
    break;
  }
}

void WorldTab::on_socket_ready_read()
{
  client.read(socket, document);
}
