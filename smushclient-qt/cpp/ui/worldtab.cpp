#include "rust/cxx.h"
#include "../settings.h"
#include "worldtab.h"
#include "ui_worldtab.h"
#include "worldprefs.h"
#include <QtGui/QAction>
#include <QtGui/QDesktopServices>
#include <QtGui/QFont>
#include <QtGui/QFontDatabase>
#include <QtGui/QPalette>
#include <QtCore/QUrl>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMenu>
#include <QtWidgets/QErrorMessage>

static void setColors(QWidget *widget, QColor foreground, QColor background)
{
  QPalette palette = QPalette(widget->palette());
  palette.setColor(QPalette::Text, foreground);
  palette.setColor(QPalette::Base, background);
  palette.setColor(QPalette::AlternateBase, background);
  widget->setPalette(palette);
}

WorldTab::WorldTab(QWidget *parent)
    : QSplitter(parent), ui(new Ui::WorldTab), socket(this), defaultFont(QFontDatabase::systemFont(QFontDatabase::FixedFont)), document(&socket)
{
  ui->setupUi(this);
  defaultFont.setPointSize(12);
  document.setUI(ui->output, ui->input);
  connect(&socket, &QTcpSocket::readyRead, this, &WorldTab::readFromSocket);
}

WorldTab::~WorldTab()
{
  delete ui;
}

// Public methods

void WorldTab::createWorld() &
{
  const QString defaultFontFamily = defaultFont.family();
  const int defaultFontSize = defaultFont.pointSize();
  client.populateWorld(world);
  world.setInputFont(defaultFontFamily);
  world.setInputFontSize(defaultFontSize);
  world.setOutputFont(defaultFontFamily);
  world.setOutputFontSize(defaultFontSize);
  client.setWorld(world);
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
  }
  catch (const rust::Error &e)
  {
    QErrorMessage::qtHandler()->showMessage(QString(e.what()));
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
  WorldPrefs *prefs = new WorldPrefs(&world, this);
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
    QErrorMessage::qtHandler()->showMessage(QString(e.what()));
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
    QErrorMessage::qtHandler()->showMessage(QString(e.what()));
    return QString();
  }
}

const QString WorldTab::title() const noexcept
{
  return world.getName();
}

// Private methods

void WorldTab::applyWorld()
{
  document.setPalette(client.palette());
  setColors(ui->input, world.getInputColorsForeground(), world.getInputColorsBackground());
  setColors(ui->output, world.getAnsiColors7(), world.getAnsiColors0());
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
  client.read(socket, document);
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

  const QString last = action.last(1);
  if (last == "\x17")
    QDesktopServices::openUrl(QUrl(action));
  else if (last == "\x18")
    ui->input->setText(action);
  else
    sendCommand(action);
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
  for (QString prompt : prompts.split("|"))
    menu.addAction(prompt);

  const QAction *chosen = menu.exec(mouse);
  if (chosen == nullptr)
    return;

  sendCommand(chosen->text());
}
