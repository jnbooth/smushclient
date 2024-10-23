#include "worldtab.h"
#include <string>
#include <QtCore/QUrl>
#include <QtGui/QAction>
#include <QtGui/QDesktopServices>
#include <QtGui/QFontDatabase>
#include <QtGui/QPalette>
#include <QtWidgets/QErrorMessage>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMenu>
#include "pluginsdialog.h"
#include "ui_worldtab.h"
#include "worldprefs.h"
#include "../bridge/document.h"
#include "../environment.h"
#include "../scripting/hotspot.h"
#include "../scripting/qlua.h"
#include "../scripting/scriptapi.h"
#include "../settings.h"
#include "../spans.h"
#include "rust/cxx.h"

using std::nullopt;
using std::string;

// Private utilities

void setColors(QWidget *widget, const QColor &foreground, const QColor &background)
{
  QPalette palette(widget->palette());
  palette.setColor(QPalette::Text, foreground);
  palette.setColor(QPalette::Base, background);
  palette.setColor(QPalette::AlternateBase, background);
  widget->setPalette(palette);
}

inline void showRustError(const rust::Error &e)
{
  QErrorMessage::qtHandler()->showMessage(QString::fromUtf8(e.what()));
}

// Public methods

WorldTab::WorldTab(QWidget *parent)
    : QSplitter(parent),
      ui(new Ui::WorldTab),
      defaultFont(QFontDatabase::systemFont(QFontDatabase::FixedFont)),
      onDragMove(nullopt),
      onDragRelease(nullptr),
      resizeTimerId(0)
{
  ui->setupUi(this);
  ui->input->setFocus();
  defaultFont.setPointSize(12);
  socket = new QTcpSocket(this);
  api = new ScriptApi(this);
  document = new Document(this, api);
  connect(socket, &QTcpSocket::readyRead, this, &WorldTab::readFromSocket);
  connect(socket, &QTcpSocket::connected, this, &WorldTab::onConnect);
  connect(socket, &QTcpSocket::disconnected, this, &WorldTab::onDisconnect);
}

WorldTab::~WorldTab()
{
  OnPluginClose onPluginClose;
  api->sendCallback(onPluginClose);
  disconnect(socket, nullptr, nullptr, nullptr);
  delete api;
  delete ui;
}

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
  openLog();
  loadPlugins();
  applyWorld();
}

void WorldTab::onTabSwitch(bool active) const
{
  if (!active)
  {
    OnPluginLoseFocus onLoseFocus;
    api->sendCallback(onLoseFocus);
    return;
  }
  ui->input->focusWidget();
  OnPluginGetFocus onGetFocus;
  api->sendCallback(onGetFocus);
}

void WorldTab::openPluginsDialog()
{
  PluginsDialog dialog(client, this);
  connect(&dialog, &PluginsDialog::reinstallClicked, this, &WorldTab::loadPlugins);
  if (dialog.exec() != QDialog::Accepted)
    return;
  loadPlugins();
}

bool WorldTab::openWorld(const QString &filename) &
{
  try
  {
    client.loadWorld(filename, world);
  }
  catch (const rust::Error &e)
  {
    showRustError(e);
    return false;
  }
  openLog();
  loadPlugins();
  try
  {
    client.loadVariables(filename + QStringLiteral(".vars"));
  }
  catch (const rust::Error &e)
  {
    showRustError(e);
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

  if (!saveWorldAndState(filePath))
    return QString();

  return filePath;
}

QString WorldTab::saveWorldAsNew(const QString &saveFilter)
{
  const QString title = tr("Save as");
  const QString path = QFileDialog::getSaveFileName(
      this,
      tr("Save as"),
      QStringLiteral(WORLDS_DIR "/%1").arg(world.getName()),
      saveFilter);
  if (path.isEmpty())
    return path;

  if (!saveWorldAndState(path))
    return QString();

  filePath = path;
  return filePath;
}

void WorldTab::setOnDragMove(CallbackTrigger &&trigger)
{
  onDragMove.emplace(std::move(trigger));
}

void WorldTab::setOnDragRelease(Hotspot *hotspot)
{
  onDragRelease = hotspot;
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

// Protected overrides

void WorldTab::mouseMoveEvent(QMouseEvent *)
{
  if (onDragMove) [[unlikely]]
    onDragMove->trigger();
}

void WorldTab::leaveEvent(QEvent *)
{
  finishDrag();
}

void WorldTab::mouseReleaseEvent(QMouseEvent *)
{
  finishDrag();
}

void WorldTab::resizeEvent(QResizeEvent *event)
{
  if (resizeTimerId)
    killTimer(resizeTimerId);
  resizeTimerId = startTimer(1000);
  QSplitter::resizeEvent(event);
}

void WorldTab::timerEvent(QTimerEvent *event)
{
  const int id = event->timerId();
  killTimer(id);
  if (resizeTimerId != id)
    return;
  resizeTimerId = 0;
  OnPluginWorldOutputResized onWorldOutputResized;
  api->sendCallback(onWorldOutputResized);
}

// Private methods

void WorldTab::applyWorld() const
{
  document->setPalette(client.palette());
  setColors(ui->background, world.getAnsi7(), world.getAnsi0());

  MudInput *input = ui->input;
  setColors(input, world.getInputTextColour(), world.getInputBackgroundColour());
  input->setMaxLogSize(world.getHistoryLines());
  const QFont &inputFont =
      world.getUseDefaultInputFont()
          ? defaultFont
          : QFont(world.getInputFont(), world.getInputFontHeight());
  input->setFont(inputFont);

  MudBrowser *output = ui->output;
  const QFont &outputFont =
      world.getUseDefaultOutputFont()
          ? defaultFont
          : QFont(world.getOutputFont(), world.getOutputFontHeight());
  output->setFont(outputFont);

  api->applyWorld(world);
}

void WorldTab::connectToHost() const
{
  if (socket->isOpen())
    return;

  socket->connectToHost(world.getSite(), (uint16_t)world.getPort());
}

inline void WorldTab::finishDrag()
{
  onDragMove.reset();
  if (onDragRelease) [[unlikely]]
  {
    onDragRelease->finishDrag();
    onDragRelease = nullptr;
  }
}

void WorldTab::openLog()
{
  try
  {
    client.openLog();
  }
  catch (const rust::Error &e)
  {
    showRustError(e);
  }
}

bool WorldTab::saveWorldAndState(const QString &path) const
{
  OnPluginWorldSave onWorldSave;
  api->sendCallback(onWorldSave);
  try
  {
    client.saveWorld(path);
  }
  catch (const rust::Error &e)
  {
    showRustError(e);
    return false;
  }
  OnPluginSaveState onSaveState;
  api->sendCallback(onSaveState);
  try
  {
    client.saveVariables(path + QStringLiteral(".vars"));
  }
  catch (const rust::Error &e)
  {
    showRustError(e);
  }
  return true;
}

// Private slots

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

bool WorldTab::loadPlugins()
{
  const QStringList errors = client.loadPlugins();
  if (!errors.empty())
  {
    QErrorMessage::qtHandler()->showMessage(errors.join(QChar::fromLatin1('\n')));
    return false;
  }
  QStringList plugins = client.pluginScripts();
  api->initializeScripts(plugins);
  return true;
}

void WorldTab::onConnect()
{
  api->setOpen(true);
  OnPluginConnect onConnect;
  api->sendCallback(onConnect);
}

void WorldTab::onDisconnect()
{
  api->setOpen(false);
  OnPluginDisconnect onDisconnect;
  api->sendCallback(onDisconnect);
}

void WorldTab::readFromSocket()
{
  const ActionSource currentSource = api->setSource(ActionSource::TriggerFired);
  client.read(*socket, *document);
  api->setSource(currentSource);
}

void WorldTab::on_input_submitted(const QString &text)
{
  const auto aliasOutcome = client.alias(text, *document);

  if (!(aliasOutcome & (uint8_t)AliasOutcome::Remember))
    ui->input->forgetLast();

  if (!(aliasOutcome & (uint8_t)AliasOutcome::Send))
  {
    ui->input->clear();
    return;
  }

  QByteArray bytes = text.toUtf8();
  OnPluginCommand onCommand(bytes);
  api->sendCallback(onCommand);
  if (onCommand.discarded())
  {
    ui->input->clear();
    return;
  }
  OnPluginCommandEntered onCommandEntered(bytes);
  api->sendCallback(onCommandEntered);
  if (bytes.size() == 1)
  {
    switch (bytes.front())
    {
    case '\t':
      ui->input->clear();
      return;
    case '\r':
      return;
    }
  }
  ui->input->clear();
  api->SendNoEcho(bytes);
}

void WorldTab::on_input_textChanged()
{
  OnPluginCommandChanged onCommandChanged;
  api->sendCallback(onCommandChanged);
}

class AnchorCallback : public PluginCallback
{
public:
  AnchorCallback(const string &callback, const QString &arg)
      : callback(callback.data()),
        arg(arg) {}

  inline constexpr const char *name() const noexcept override { return callback; }
  inline constexpr ActionSource source() const noexcept override { return ActionSource::UserMenuAction; }

  int pushArguments(lua_State *L) const override
  {
    qlua::pushQString(L, arg);
    return 1;
  }

private:
  const char *callback;
  const QString &arg;
};

void WorldTab::on_output_anchorClicked(const QUrl &url)
{
  QString action = url.toString(QUrl::None);
  if (action.isEmpty())
    return;

  switch (decodeLink(action))
  {
  case SendTo::Internet:
    QDesktopServices::openUrl(QUrl(action));
    return;
  case SendTo::World:
    break;
  case SendTo::Input:
    ui->input->setText(action);
    return;
  }

  int delimIndex, fnIndex;
  if (
      action.first(2) == QStringLiteral("!!") &&
      action.back() == QChar::fromLatin1(')') &&
      (delimIndex = action.indexOf(QChar::fromLatin1(':'))) != -1 &&
      (fnIndex = action.indexOf(QChar::fromLatin1('('), delimIndex)) != -1)
  {
    const QString pluginID = action.sliced(2, delimIndex - 2);
    const string functionName = action.sliced(delimIndex + 1, fnIndex - delimIndex - 1).toStdString();
    const QString arg = action.sliced(fnIndex + 1, action.size() - fnIndex - 2);
    AnchorCallback callback(functionName, arg);
    api->sendCallback(callback, pluginID);
    return;
  }

  const auto aliasOutcome = client.alias(action, *document);

  if (!(aliasOutcome & (uint8_t)AliasOutcome::Send))
    return;

  QByteArray bytes = action.toUtf8();
  OnPluginCommand onCommand(bytes);
  api->sendCallback(onCommand);
  if (onCommand.discarded())
    return;

  api->SendNoEcho(bytes);
}

void WorldTab::on_output_customContextMenuRequested(const QPoint &pos)
{
  const QTextCharFormat format = ui->output->cursorForPosition(pos).charFormat();
  const QPoint mouse = ui->output->mapToGlobal(pos);
  const QString prompts = getPrompts(format);
  if (prompts.isEmpty())
  {
    ui->output->createStandardContextMenu(mouse)->exec(mouse);
    return;
  }
  QMenu menu(ui->output);
  for (const QString &prompt : prompts.split(QStringLiteral("|")))
    menu.addAction(prompt);

  const QAction *chosen = menu.exec(mouse);
  if (!chosen)
    return;

  api->Send(chosen->text());
}
