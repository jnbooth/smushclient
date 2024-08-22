#include "worldtab.h"
#include "ui_worldtab.h"
#include <QtWidgets/QDialog>
#include <QtGui/QFont>
#include "worldprefs.h"

WorldTab::WorldTab(QWidget *parent)
    : QSplitter(parent), ui(new Ui::WorldTab), socket(this)
{
  ui->setupUi(this);
  document.setBrowser(ui->output);
  socket.setObjectName("socket");
  connect(&socket, &QTcpSocket::readyRead, this, &WorldTab::on_readyRead);
}

WorldTab::~WorldTab()
{
  delete ui;
}

const QString &WorldTab::title() const
{
  return world.getName();
}

void WorldTab::connectToHost()
{
  if (socket.isOpen())
  {
    return;
  }
  socket.connectToHost(world.getSite(), (quint16)world.getPort());
}

void WorldTab::openPreferences()
{
  WorldPrefs *prefs = new WorldPrefs(&world, this);
  prefs->setAttribute(Qt::WA_DeleteOnClose, true);
  connect(prefs, &QDialog::finished, this, &WorldTab::on_finished);
  prefs->open();
}

void WorldTab::createWorld()
{
  QFont defaultFont = QFont();
  defaultFont.setStyleHint(QFont::System);
  QString fontFamily = defaultFont.family();
  client.populateWorld(world);
  world.setOutputFont(fontFamily);
  world.setInputFont(fontFamily);
  client.setWorld(world);
}

bool WorldTab::openWorld(const QString &filename)
{
  if (!client.loadWorld(filename, world))
  {
    return false;
  }
  connectToHost();
  return true;
}

void WorldTab::on_finished(int result)
{
  switch (result)
  {
  case QDialog::Accepted:
    client.setWorld(world);
    connectToHost();
    break;
  case QDialog::Rejected:
    client.populateWorld(world);
    break;
  }
}

void WorldTab::on_readyRead()
{
  client.read(socket, document);
}

void WorldTab::on_input_returnPressed()
{
  QString input = ui->input->text();
  input.append("\r\n");
  socket.write(input.toLocal8Bit());
  ui->input->clear();
}
