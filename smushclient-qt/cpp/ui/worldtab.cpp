#include "worldtab.h"
#include <QtCore/QUrl>
#include <QtGui/QAction>
#include <QtGui/QDesktopServices>
#include <QtGui/QFontDatabase>
#include <QtGui/QPalette>
#include <QtWidgets/QErrorMessage>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMenu>
#include "ui_worldtab.h"
#include "worldprefs.h"
#include "../bridge/document.h"
#include "../bridge/scriptengine.h"
#include "../scripting/scriptapi.h"
#include "../settings.h"
#include "rust/cxx.h"

void setColors(QWidget *widget, const QColor &foreground, const QColor &background)
{
  QPalette palette = QPalette(widget->palette());
  palette.setColor(QPalette::Text, foreground);
  palette.setColor(QPalette::Base, background);
  palette.setColor(QPalette::AlternateBase, background);
  widget->setPalette(palette);
}

WorldTab::WorldTab(QWidget *parent)
    : QSplitter(parent),
      ui(new Ui::WorldTab),
      defaultFont(QFontDatabase::systemFont(QFontDatabase::FixedFont))
{
  ui->setupUi(this);
  defaultFont.setPointSize(12);
  socket = new QTcpSocket(this);
  api = new ScriptApi(this);
  document = new Document(this, api);
  connect(socket, &QTcpSocket::readyRead, this, &WorldTab::readFromSocket);
}

WorldTab::~WorldTab()
{
  delete api;
  delete ui;
}

// Public methods

void WorldTab::createWorld() &
{
  const QString defaultFontFamily = defaultFont.family();
  const int defaultFontHeight = defaultFont.pointSize();
  client.populateWorld(world);
  world.setInputFont(defaultFontFamily);
  world.setInputFontHeight(defaultFontHeight);
  world.setOutputFont(defaultFontFamily);
  world.setOutputFontHeight(defaultFontHeight);
  client.setWorld(world);
  document->scriptEngine.initializeScripts(client.pluginScripts());
  applyWorld();
}

void WorldTab::focusInput() const
{
  ui->input->focusWidget();
}

bool WorldTab::openWorld(const QString &filename) &
{
  try
  {
    client.loadWorld(filename, world);
    document->scriptEngine.initializeScripts(client.pluginScripts());
  }
  catch (const rust::Error &e)
  {
    QErrorMessage::qtHandler()->showMessage(QString::fromUtf8(e.what()));
    return false;
  }

  Settings().addRecentFile(filename);
  filePath = QString(filename);
  applyWorld();
  connectToHost();
  return true;
}

void WorldTab::openWorldSettings() &
{
  WorldPrefs *prefs = new WorldPrefs(world, this);
  prefs->setAttribute(Qt::WA_DeleteOnClose, true);
  connect(prefs, &QDialog::finished, this, &WorldTab::finalizeWorldSettings);
  prefs->open();
}

QString WorldTab::saveWorld(const QString &saveFilter)
{
  if (filePath.isEmpty())
    return saveWorldAsNew(saveFilter);

  try
  {
    client.saveWorld(filePath);
    return filePath;
  }
  catch (const rust::Error &e)
  {
    QErrorMessage::qtHandler()->showMessage(QString::fromUtf8(e.what()));
    return QString();
  }
}

QString WorldTab::saveWorldAsNew(const QString &saveFilter)
{
  const QString title = tr("Save as");
  const QString path = QFileDialog::getSaveFileName(this, title, world.getName(), saveFilter);
  if (path.isEmpty())
    return path;

  try
  {
    client.saveWorld(path);
    filePath = path;
    return filePath;
  }
  catch (const rust::Error &e)
  {
    QErrorMessage::qtHandler()->showMessage(QString::fromUtf8(e.what()));
    return QString();
  }
}

const QString WorldTab::title() const noexcept
{
  return world.getName();
}

bool WorldTab::updateWorld()
{
  if (client.setWorld(world))
  {
    applyWorld();
    return true;
  }
  client.populateWorld(world);
  return false;
}

// Private methods

void WorldTab::applyWorld()
{
  document->setPalette(client.palette());
  setColors(ui->input, world.getInputTextColour(), world.getInputBackgroundColour());
  setColors(ui->output, world.getAnsi7(), world.getAnsi0());
  if (world.getUseDefaultInputFont())
    ui->input->setFont(defaultFont);
  else
    ui->input->setFont(QFont(world.getInputFont(), world.getInputFontHeight()));

  if (world.getUseDefaultOutputFont())
    ui->output->setFont(defaultFont);
  else
    ui->output->setFont(QFont(world.getOutputFont(), world.getOutputFontHeight()));
  api->applyWorld(world);
}

void WorldTab::connectToHost()
{
  if (socket->isOpen())
    return;

  socket->connectToHost(world.getSite(), (quint16)world.getPort());
}

void WorldTab::sendCommand(const QString &command)
{
  api->echo(command);
  QByteArray bytes = command.toLocal8Bit();
  bytes.append("\r\n");
  socket->write(bytes);
}

// Slots

void WorldTab::finalizeWorldSettings(int result)
{
  switch (result)
  {
  case QDialog::Accepted:
    client.setWorld(world);
    applyWorld();
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
  client.read(*socket, *document);
}

void WorldTab::on_input_returnPressed()
{
  const QString input = ui->input->text();
  sendCommand(input);
  ui->input->clear();
}

void WorldTab::on_output_anchorClicked(const QUrl &url)
{
  const QString action = url.toString(QUrl::None);
  if (action.isEmpty())
    return;

  switch (action.back().unicode())
  {
  case 17:
    QDesktopServices::openUrl(QUrl(action));
    break;
  case 18:
    ui->input->setText(action);
    break;
  default:
    sendCommand(action);
  }
}

void WorldTab::on_output_customContextMenuRequested(const QPoint &pos)
{
  const QTextCharFormat format = ui->output->cursorForPosition(pos).charFormat();
  const QPoint mouse = ui->output->mapToGlobal(pos);
  if (!format.hasProperty(QTextCharFormat::UserProperty))
  {
    ui->output->createStandardContextMenu(mouse)->exec(mouse);
    return;
  }
  const QString prompts = format.property(QTextCharFormat::UserProperty).value<QString>();
  QMenu menu(ui->output);
  for (const QString &prompt : prompts.split(QStringLiteral("|")))
    menu.addAction(prompt);

  const QAction *chosen = menu.exec(mouse);
  if (chosen == nullptr)
    return;

  sendCommand(chosen->text());
}
