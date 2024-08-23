#include "worldtab.h"
#include "ui_worldtab.h"
#include "worldprefs.h"
#include <QtGui/QAction>
#include <QtGui/QDesktopServices>
#include <QtGui/QFont>
#include <QtCore/QUrl>
#include <QtWidgets/QDialog>
#include <QtWidgets/QMenu>

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
  defaultFont.setStyleHint(QFont::Monospace);
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
