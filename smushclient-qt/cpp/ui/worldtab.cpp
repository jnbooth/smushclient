#include "worldtab.h"
#include <string>
#include <QtCore/QUrl>
#include <QtGui/QAction>
#include <QtGui/QDesktopServices>
#include <QtGui/QFontDatabase>
#include <QtGui/QPalette>
#include <QtNetwork/QNetworkProxy>
#include <QtWidgets/QErrorMessage>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>
#include "pluginsdialog.h"
#include "ui_worldtab.h"
#include "worldprefs.h"
#include "../bridge/document.h"
#include "../environment.h"
#include "../hotkeys.h"
#include "../scripting/hotspot.h"
#include "../scripting/qlua.h"
#include "../components/mudscrollbar.h"
#include "../scripting/scriptapi.h"
#include "../settings.h"
#include "../spans.h"
#include "rust/cxx.h"

using std::nullopt;
using std::string;

constexpr Qt::KeyboardModifiers numpadMods = Qt::KeyboardModifier::ControlModifier | Qt::KeyboardModifier::MetaModifier;

// Private utilities

void setColors(QWidget *widget, const QColor &foreground, const QColor &background)
{
  QPalette palette(widget->palette());
  palette.setColor(QPalette::Text, foreground);
  palette.setColor(QPalette::Base, background);
  palette.setColor(QPalette::AlternateBase, background);
  widget->setPalette(palette);
}

void setColor(QWidget *widget, QPalette::ColorRole role, const QColor &color)
{
  QPalette palette(widget->palette());
  palette.setColor(role, color);
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
      client(),
      world(),
      defaultFont(QFontDatabase::systemFont(QFontDatabase::FixedFont)),
      filePath(),
      flushTimerId(-1),
      handleKeypad(false),
      initialized(false),
      inputCopyAvailable(false),
      onDragMove(nullopt),
      onDragRelease(nullptr),
      outputCopyAvailable(false),
      queuedConnect(false),
      resizeTimerId(-1),
      splitter(),
      useSplitter(false),
      worldScriptWatcher(this)
{
  resizeTimerId = startTimer(1000);
  ui->setupUi(this);
  ui->input->setFocus();
  defaultFont.setPointSize(12);
  socket = new QTcpSocket(this);
  api = new ScriptApi(this);
  document = new Document(this, api);
  connect(socket, &QTcpSocket::readyRead, this, &WorldTab::readFromSocket);
  connect(socket, &QTcpSocket::connected, this, &WorldTab::onConnect);
  connect(socket, &QTcpSocket::disconnected, this, &WorldTab::onDisconnect);
  connect(&worldScriptWatcher, &QFileSystemWatcher::fileChanged, this, &WorldTab::confirmReloadWorldScript);

  const Settings settings;
  setColors(ui->input, settings.inputForeground(), settings.inputBackground());
  ui->input->setFont(settings.inputFont());
  ui->output->setFont(settings.outputFont());

  const QTextEdit::LineWrapMode wrapMode =
      settings.outputWrapping()
          ? QTextEdit::LineWrapMode::WidgetWidth
          : QTextEdit::LineWrapMode::NoWrap;
  ui->output->setLineWrapMode(wrapMode);
}

WorldTab::~WorldTab()
{
  OnPluginClose onPluginClose;
  api->sendCallback(onPluginClose);
  disconnect(socket, nullptr, nullptr, nullptr);
  delete api;
  delete ui;
}

AvailableCopy WorldTab::availableCopy() const
{
  if (outputCopyAvailable)
    return AvailableCopy::Output;
  if (inputCopyAvailable)
    return AvailableCopy::Input;
  return AvailableCopy::None;
}

void WorldTab::closeLog()
{
  client.closeLog();
}

QTextEdit *WorldTab::copyableEditor() const
{
  if (outputCopyAvailable)
    return ui->output;
  if (inputCopyAvailable)
    return ui->input;
  return nullptr;
}

void WorldTab::createWorld() &
{
  client.populateWorld(world);
}

void WorldTab::connectToHost()
{
  if (!initialized)
  {
    queuedConnect = true;
    return;
  }

  if (socket->state() != QAbstractSocket::SocketState::UnconnectedState)
    return;

  socket->connectToHost(world.getSite(), (uint16_t)world.getPort());
}

void WorldTab::disconnectFromHost()
{
  socket->disconnectFromHost();
}

void WorldTab::editWorldScript()
{
  const QString &scriptPath = world.getWorldScript();
  if (!QDesktopServices::openUrl(QUrl::fromLocalFile(scriptPath)))
    QErrorMessage::qtHandler()->showMessage(tr("Failed to open file: %1").arg(scriptPath));
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
  filePath = filename;
  return true;
}

bool WorldTab::openWorldSettings()
{
  WorldPrefs prefs(world, this);
  if (prefs.exec() == QDialog::Accepted)
  {
    client.setWorld(world);
    applyWorld();
    return true;
  }
  client.populateWorld(world);
  return false;
}

void WorldTab::reloadWorldScript() const
{
  api->reloadWorldScript(world.getWorldScript());
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

void WorldTab::start()
{
  setupWorldScriptWatcher();
  if (Settings().loggingEnabled())
    openLog();

  if (!filePath.isEmpty())
  {
    try
    {
      client.loadVariables(filePath + QStringLiteral(".vars"));
    }
    catch (const rust::Error &e)
    {
      showRustError(e);
    }
  }

  loadPlugins();

  applyWorld();
  connectToHost();
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

// Public slots

void WorldTab::onInputBackgroundChanged(const QColor &color)
{
  setColor(ui->input, QPalette::ColorRole::Base, color);
}

void WorldTab::onInputFontChanged(const QFont &font)
{
  ui->input->setFont(font);
}

void WorldTab::onInputForegroundChanged(const QColor &color)
{
  setColor(ui->input, QPalette::ColorRole::Text, color);
}

void WorldTab::onOutputFontChanged(const QFont &font)
{
  ui->output->setFont(font);
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

void WorldTab::keyPressEvent(QKeyEvent *event)
{
  const Qt::KeyboardModifiers modifiers = event->modifiers();
  if (!handleKeypad || !modifiers.testFlag(Qt::KeyboardModifier::KeypadModifier)) [[likely]]
  {
    QSplitter::keyPressEvent(event);
    return;
  }
  QString action = hotkeys::numpad(world, Qt::Key(event->key()), modifiers.testAnyFlags(numpadMods));
  if (action.isEmpty())
    return;
  sendCommand(action, CommandSource::Hotkey);
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
  if (id == flushTimerId)
  {
    flushTimerId = 0;
    const ActionSource currentSource = api->setSource(ActionSource::TriggerFired);
    client.flush(*document);
    api->setSource(currentSource);
    return;
  }
  if (id != resizeTimerId)
    return;
  resizeTimerId = 0;
  initialized = true;
  OnPluginWorldOutputResized onWorldOutputResized;
  api->sendCallback(onWorldOutputResized);
  if (queuedConnect)
  {
    queuedConnect = false;
    connectToHost();
  }
}

// Private methods

void WorldTab::applyWorld()
{
  handleKeypad = world.getNumpadEnable();
  ui->input->setIgnoreKeypad(handleKeypad);
  ui->output->setIgnoreKeypad(handleKeypad);
  document->setPalette(client.palette());
  setColors(ui->background, world.getAnsi7(), world.getAnsi0());
  if (world.getUseProxy())
    socket->setProxy(QNetworkProxy(
        QNetworkProxy::ProxyType::Socks5Proxy,
        world.getProxyServer(),
        world.getProxyPort(),
        world.getProxyUsername(),
        world.getProxyPassword()));
  else
    socket->setProxy(QNetworkProxy::NoProxy);
  if (world.getSaveWorldAutomatically())
    saveWorldAndState(filePath);

  api->applyWorld(world);
  updateWorldScript();
  if (!world.getEnableCommandStack())
  {
    useSplitter = false;
    return;
  }
  const QChar splitOn(world.getCommandStackCharacter());
  if (splitOn == u']')
  {
    QChar chars[] = {u'[', u'\n', u'\\', splitOn, u']'};
    splitter.setPattern(QString(chars, 5));
  }
  else
  {
    QChar chars[] = {u'[', u'\n', splitOn, u']'};
    splitter.setPattern(QString(chars, 4));
  }
  useSplitter = true;
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

bool WorldTab::sendCommand(const QString &command, CommandSource source)
{
  QTextCursor cursor(ui->output->document());
  cursor.movePosition(QTextCursor::End);
  int echoStart = cursor.position();
  api->echo(command);
  int echoEnd = cursor.position();

  const auto aliasOutcome = client.alias(command, source, *document);

  if (!aliasOutcome.testFlag(AliasOutcome::Display))
  {
    cursor.setPosition(echoStart);
    cursor.setPosition(echoEnd, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
  }

  if (aliasOutcome.testFlag(AliasOutcome::Remember))
    ui->input->remember(command);

  if (!aliasOutcome.testFlag(AliasOutcome::Send))
    return true;

  QByteArray bytes = command.toUtf8();
  OnPluginCommand onCommand(bytes);
  api->sendCallback(onCommand);
  if (onCommand.discarded())
    return true;
  OnPluginCommandEntered onCommandEntered(bytes);
  api->sendCallback(onCommandEntered);
  if (bytes.size() == 1)
  {
    switch (bytes.front())
    {
    case '\t':
      return true;
    case '\r':
      return false;
    }
  }
  api->SendNoEcho(bytes);
  return true;
}

void WorldTab::setupWorldScriptWatcher()
{
  const QStringList watchedScripts = worldScriptWatcher.files();
  if (!watchedScripts.isEmpty())
    worldScriptWatcher.removePaths(watchedScripts);
  const QString &worldScriptPath = world.getWorldScript();
  if (!worldScriptPath.isEmpty())
    worldScriptWatcher.addPath(worldScriptPath);
}

void WorldTab::updateWorldScript()
{
  const QString &worldScriptPath = world.getWorldScript();
  const QStringList watchedScripts = worldScriptWatcher.files();
  if (watchedScripts.value(0) == worldScriptPath)
    return;
  if (!watchedScripts.isEmpty())
    worldScriptWatcher.removePaths(watchedScripts);
  if (!worldScriptPath.isEmpty())
    worldScriptWatcher.addPath(worldScriptPath);
  confirmReloadWorldScript(worldScriptPath);
}

// Private slots

void WorldTab::confirmReloadWorldScript(const QString &worldScriptPath)
{
  QFileInfo info(worldScriptPath);
  if (!info.isFile() || !info.isReadable())
    return;
  switch (world.getScriptReloadOption())
  {
  case ScriptRecompile::Always:
    break;
  case ScriptRecompile::Never:
    return;
  case ScriptRecompile::Confirm:
    if (QMessageBox::question(this, tr("World script changed"), tr("Would you like to reload the world script?")) != QMessageBox::StandardButton::Yes)
      return;
  }
  api->reloadWorldScript(worldScriptPath);
}

bool WorldTab::loadPlugins()
{
  const QStringList errors = client.loadPlugins();
  if (!errors.empty())
  {
    QErrorMessage::qtHandler()->showMessage(errors.join(u'\n'));
    return false;
  }
  api->initializePlugins(client.pluginScripts());
  return true;
}

void WorldTab::onConnect()
{
  client.handleConnect(*socket);
  api->setOpen(true);
  OnPluginConnect onConnect;
  api->sendCallback(onConnect);
}

void WorldTab::onDisconnect()
{
  client.handleDisconnect();
  api->setOpen(false);
  OnPluginDisconnect onDisconnect;
  api->sendCallback(onDisconnect);
}

void WorldTab::readFromSocket()
{
  const ActionSource currentSource = api->setSource(ActionSource::TriggerFired);
  client.read(*socket, *document);
  api->setSource(currentSource);
  if (client.hasOutput())
    flushTimerId = startTimer(2000);
  else
    flushTimerId = -1;
}

void WorldTab::on_input_copyAvailable(bool available)
{
  inputCopyAvailable = available;
  emit copyAvailable(availableCopy());
}

void WorldTab::on_input_submitted(const QString &text)
{
  ui->output->verticalScrollBar()->setPaused(false);

  const QStringList commands = useSplitter ? text.split(splitter) : text.split(u'\n');

  bool eraseInput = commands.length() > 1;

  for (const QString &command : commands)
    eraseInput = sendCommand(command, CommandSource::User) || eraseInput;

  if (eraseInput)
    ui->input->clear();
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
      action.back() == u')' &&
      (delimIndex = action.indexOf(u':')) != -1 &&
      (fnIndex = action.indexOf(u'(', delimIndex)) != -1)
  {
    const QString pluginID = action.sliced(2, delimIndex - 2);
    const string functionName = action.sliced(delimIndex + 1, fnIndex - delimIndex - 1).toStdString();
    const QString arg = action.sliced(fnIndex + 1, action.size() - fnIndex - 2);
    AnchorCallback callback(functionName, arg);
    api->sendCallback(callback, pluginID);
    return;
  }

  sendCommand(action, CommandSource::Hotkey);
}

void WorldTab::on_output_copyAvailable(bool available)
{
  outputCopyAvailable = available;
  emit copyAvailable(availableCopy());
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
