#include "worldtab.h"
#include "ui_worldtab.h"
#include "worldprefs.h"
#include <QtGui/QAction>
#include <QtGui/QDesktopServices>
#include <QtGui/QFont>
#include <QtGui/QFontDatabase>
#include <QtCore/QUrl>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMenu>

WorldTab::WorldTab(QWidget *parent)
    : QSplitter(parent), ui(new Ui::WorldTab), socket(this), defaultFont(QFontDatabase::systemFont(QFontDatabase::FixedFont))
{
  ui->setupUi(this);
  defaultFont.setPointSize(12);
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
  QString defaultFontFamily = defaultFont.family();
  int defaultFontSize = defaultFont.pointSize();
  client.populateWorld(world);
  world.setInputFont(defaultFontFamily);
  world.setInputFontSize(defaultFontSize);
  world.setOutputFont(defaultFontFamily);
  world.setOutputFontSize(defaultFontSize);
  client.setWorld(world);
  applyWorld();
}

bool WorldTab::openWorld(const QString &filename)
{
  if (!client.loadWorld(filename, world))
  {
    return false;
  }
  filePath = filename;
  applyWorld();
  connectToHost();
  return true;
}

void WorldTab::applyWorld()
{
  if (world.getUseDefaultInputFont())
  {
    ui->input->setFont(defaultFont);
  }
  else
  {
    ui->input->setFont(QFont(world.getInputFont(), world.getInputFontSize()));
  }
  if (world.getUseDefaultOutputFont())
  {
    ui->output->setFont(defaultFont);
  }
  else
  {
    ui->output->setFont(QFont(world.getOutputFont(), world.getOutputFontSize()));
  }
}

bool WorldTab::saveWorld(const QString &saveFilter)
{
  if (!filePath.isEmpty())
  {
    return client.saveWorld(filePath);
  }
  return saveWorldAsNew(saveFilter);
}

bool WorldTab::saveWorldAsNew(const QString &saveFilter)
{
  QString path = QFileDialog::getSaveFileName(this, tr("Save as"), world.getName(), saveFilter);
  if (path.isEmpty())
  {
    return false;
  }
  filePath = path;
  return client.saveWorld(filePath);
}

void WorldTab::sendCommand(const QString &command)
{
  QByteArray bytes = command.toLocal8Bit();
  bytes.append("\r\n");
  socket.write(bytes);
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
    if (world.getSite().isEmpty())
    {
      delete this;
    }
  }
}

void WorldTab::on_readyRead()
{
  client.read(socket, document);
}

void WorldTab::on_input_returnPressed()
{
  QString input = ui->input->text();
  sendCommand(input);
  ui->input->clear();
}

void WorldTab::on_output_anchorClicked(const QUrl &url)
{
  QString action = url.toString(QUrl::None);
  if (action.isEmpty())
  {
    return;
  }
  QString last = action.last(1);
  if (last == "\x17")
  {
    QDesktopServices::openUrl(QUrl(action));
    return;
  }
  if (last == "\x18")
  {
    ui->input->setText(action);
    return;
  }
  sendCommand(action);
}

void WorldTab::on_output_customContextMenuRequested(const QPoint &pos)
{
  QTextCharFormat format = ui->output->cursorForPosition(pos).charFormat();
  QPoint mouse = ui->output->mapToGlobal(pos);
  if (!format.hasProperty(QTextCharFormat::UserProperty))
  {
    ui->output->createStandardContextMenu(mouse)->exec(mouse);
    return;
  }
  QStringList prompts = format.property(QTextCharFormat::UserProperty).value<QStringList>();
  QMenu menu = QMenu(ui->output);
  for (QString prompt : prompts)
  {
    menu.addAction(prompt);
  }
  QAction *chosen = menu.exec(mouse);
  if (chosen == NULL)
  {
    return;
  }
  sendCommand(chosen->text());
}
