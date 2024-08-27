#include "../settings.h"
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
    : QSplitter(parent), ui(new Ui::WorldTab), socket(this), defaultFont(QFontDatabase::systemFont(QFontDatabase::FixedFont)), document(&socket)
{
  ui->setupUi(this);
  defaultFont.setPointSize(12);
  document.setBrowser(ui->output);
  document.setLineEdit(ui->input);
  connect(&socket, &QTcpSocket::readyRead, this, &WorldTab::readFromSocket);
}

WorldTab::~WorldTab()
{
  delete ui;
}

// Public methods

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
    return false;

  filePath = filename;
  Settings().addRecentFile(filePath);
  applyWorld();
  connectToHost();
  return true;
}

void WorldTab::openWorldSettings()
{
  WorldPrefs *prefs = new WorldPrefs(&world, this);
  prefs->setAttribute(Qt::WA_DeleteOnClose, true);
  connect(prefs, &QDialog::finished, this, &WorldTab::finalizeWorldSettings);
  prefs->open();
}

QString WorldTab::saveWorld(const QString &saveFilter)
{
  if (filePath.isEmpty())
    return saveWorldAsNew(saveFilter);

  if (client.saveWorld(filePath))
    return filePath;

  return QString();
}

QString WorldTab::saveWorldAsNew(const QString &saveFilter)
{
  QString path = QFileDialog::getSaveFileName(this, tr("Save as"), world.getName(), saveFilter);
  if (path.isEmpty())
    return path;

  filePath = path;
  if (client.saveWorld(filePath))
    return filePath;

  return QString();
}

const QString &WorldTab::title() const
{
  return world.getName();
}

// Private methods

void WorldTab::applyWorld()
{
  if (world.getUseDefaultInputFont())
    ui->input->setFont(defaultFont);
  else
    ui->input->setFont(QFont(world.getInputFont(), world.getInputFontSize()));

  if (world.getUseDefaultOutputFont())
    ui->output->setFont(defaultFont);
  else
    ui->output->setFont(QFont(world.getOutputFont(), world.getOutputFontSize()));
}

void WorldTab::connectToHost()
{
  if (socket.isOpen())
    return;

  socket.connectToHost(world.getSite(), (quint16)world.getPort());
}

void WorldTab::sendCommand(const QString &command)
{
  QByteArray bytes = command.toLocal8Bit();
  bytes.append("\r\n");
  socket.write(bytes);
}

// Slots

void WorldTab::finalizeWorldSettings(int result)
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
      delete this;
  }
}

void WorldTab::readFromSocket()
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
    return;

  QString last = action.last(1);
  if (last == "\x17")
    QDesktopServices::openUrl(QUrl(action));
  else if (last == "\x18")
    ui->input->setText(action);
  else
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
  QMenu menu(ui->output);
  for (QString prompt : prompts)
    menu.addAction(prompt);

  QAction *chosen = menu.exec(mouse);
  if (chosen == nullptr)
    return;

  sendCommand(chosen->text());
}
