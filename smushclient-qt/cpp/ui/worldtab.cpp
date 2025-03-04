#include "worldtab.h"
#include <string>
#include <QtCore/QSaveFile>
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
#include "ui_worldtab.h"
#include "worlddetails/worlddetails.h"
#include "../bridge/document.h"
#include "../environment.h"
#include "../hotkeys.h"
#include "../localization.h"
#include "../mudstatusbar/mudstatusbar.h"
#include "../scripting/hotspot.h"
#include "../scripting/qlua.h"
#include "../components/mudscrollbar.h"
#include "../scripting/scriptapi.h"
#include "../settings.h"
#include "../spans.h"
#include "smushclient_qt/src/ffi/document.cxxqt.h"
#include "smushclient_qt/src/ffi/sender.cxxqt.h"

using std::nullopt;
using std::string;
using std::string_view;
using std::chrono::milliseconds;

constexpr Qt::KeyboardModifiers numpadMods = Qt::KeyboardModifier::ControlModifier | Qt::KeyboardModifier::MetaModifier;

// Private utilities

QString historyPath(const QString &path)
{
  return path + QStringLiteral(".history");
}

QString variablesPath(const QString &path)
{
  return path + QStringLiteral(".vars");
}

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

WorldTab::WorldTab(MudStatusBar *statusBar, Notepads *notepads, QWidget *parent)
    : QSplitter(parent),
      ui(new Ui::WorldTab),
      client(),
      world(),
      worldScriptWatcher(this)
{
  flushTimer = new QTimer(this);
  flushTimer->setInterval(milliseconds{2000});
  flushTimer->setSingleShot(true);
  connect(flushTimer, &QTimer::timeout, this, &WorldTab::flushOutput);
  resizeTimer = new QTimer(this);
  resizeTimer->setInterval(milliseconds{1000});
  resizeTimer->setSingleShot(true);
  connect(resizeTimer, &QTimer::timeout, this, &WorldTab::finishResize);
  ui->setupUi(this);
  ui->input->setFocus();
  defaultFont.setPointSize(12);
  socket = new QSslSocket(this);
  api = new ScriptApi(statusBar, notepads, this);
  document = new Document(this, api);
  connect(document, &Document::newActivity, this, &WorldTab::onNewActivity);
  connect(socket, &QSslSocket::readyRead, this, &WorldTab::readFromSocket);
  connect(socket, &QSslSocket::connected, this, &WorldTab::onSocketConnect);
  connect(socket, &QSslSocket::disconnected, this, &WorldTab::onSocketDisconnect);
  connect(socket, &QSslSocket::encrypted, this, &WorldTab::onSocketConnect);
  connect(socket, &QSslSocket::errorOccurred, this, &WorldTab::onSocketError);
  connect(&worldScriptWatcher, &QFileSystemWatcher::fileChanged, this, &WorldTab::confirmReloadWorldScript);

  const Settings settings;
  setColors(ui->input, settings.getInputForeground(), settings.getInputBackground());
  ui->input->setFont(settings.getInputFont());
  ui->output->setFont(settings.getOutputFont());

  const QTextEdit::LineWrapMode wrapMode =
      settings.getOutputWrapping()
          ? QTextEdit::LineWrapMode::WidgetWidth
          : QTextEdit::LineWrapMode::NoWrap;
  ui->output->setLineWrapMode(wrapMode);

  QTextDocument *doc = ui->output->document();
  doc->setDocumentMargin(settings.getOutputPadding());

  QTextCursor formatCursor(doc);
  formatCursor.select(QTextCursor::SelectionType::Document);
  formatCursor.mergeBlockFormat(settings.getOutputBlockFormat());
}

WorldTab::~WorldTab()
{
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

bool WorldTab::connected() const
{
  return socket->state() != QAbstractSocket::SocketState::UnconnectedState;
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

  const QString &site = world.getSite();
  const uint16_t port = world.getPort();
  const string addressString = (site + port).toStdString();
  const string_view cachedAddressString = client.getMetavariable("server/no-ssl");

  if (addressString == cachedAddressString)
  {
    socket->connectToHost(site, port);
    return;
  }

  tryingSsl = true;
  socket->connectToHostEncrypted(site, port);
  const bool didSsl = socket->waitForEncrypted();
  tryingSsl = false;

  if (didSsl || socket->error() != QSslSocket::SslHandshakeFailedError)
    return;

  tryingSsl = false;
  if (socket->state() != QAbstractSocket::SocketState::UnconnectedState)
    socket->waitForDisconnected();

  socket->connectToHost(site, port);
  client.setMetavariable("server/no-ssl", addressString);
}

void WorldTab::disconnectFromHost()
{
  manualDisconnect = true;
  socket->disconnectFromHost();
}

void WorldTab::editWorldScript()
{
  const QString &scriptPath = world.getWorldScript();
  if (!QDesktopServices::openUrl(QUrl::fromLocalFile(scriptPath)))
    QErrorMessage::qtHandler()->showMessage(tr("Failed to open file: %1").arg(scriptPath));
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
  ui->output->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
  return true;
}

bool WorldTab::openWorldSettings()
{
  WorldPrefs worlddetails(world, client, api, this);
  if (worlddetails.exec() != QDialog::Accepted)
  {
    client.populateWorld(world);
    return false;
  }
  if (worlddetails.isDirty())
    setWindowModified(true);

  if (!client.setWorld(world))
    return false;

  if (!world.getSaveWorldAutomatically())
    setWindowModified(true);

  if (Settings().getLoggingEnabled())
    openLog();

  applyWorld();
  return true;
}

bool WorldTab::promptSave()
{
  if (!isWindowModified())
    return true;
  switch (Settings().getWorldCloseBehavior())
  {
  case Settings::WorldCloseBehavior::Save:
    saveWorld();
    return true;
  case Settings::WorldCloseBehavior::Discard:
    return true;
  case Settings::WorldCloseBehavior::Confirm:
    break;
  }
  QMessageBox msgBox;
  msgBox.setText(tr("Do you want to save the changes you made to %1?").arg(world.getName()));
  msgBox.setInformativeText(tr("Your changes will be lost if you don't save them."));
  msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
  msgBox.setDefaultButton(QMessageBox::Save);
  switch (msgBox.exec())
  {
  case QMessageBox::Save:
    return !saveWorld().isEmpty();
  case QMessageBox::Discard:
    return true;
  case QMessageBox::Cancel:
    return false;
  default:
    return true;
  }
}

void WorldTab::reloadWorldScript() const
{
  api->reloadWorldScript(world.getWorldScript());
}

void WorldTab::resetAllTimers() const
{
  api->resetAllTimers();
}

QString WorldTab::saveWorld()
{
  if (filePath.isEmpty())
    return saveWorldAsNew();

  if (!saveWorldAndState(filePath))
    return QString();

  return filePath;
}

QString WorldTab::saveWorldAsNew()
{
  const QString path = QFileDialog::getSaveFileName(
      this,
      tr("Save as"),
      QStringLiteral(WORLDS_DIR) + QDir::separator() + world.getName(),
      FileFilter::world());

  if (path.isEmpty())
    return path;

  if (!saveWorldAndState(path))
    return QString();

  filePath = makePathRelative(path);
  return filePath;
}

void WorldTab::setIsActive(bool active)
{
  isActive = active;
  alertNewActivity = !active;
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

void WorldTab::setOnDragMove(CallbackTrigger &&trigger)
{
  onDragMove.emplace(std::move(trigger));
}

void WorldTab::setOnDragRelease(Hotspot *hotspot)
{
  onDragRelease = hotspot;
}

void WorldTab::setStatusBarVisible(bool visible)
{
  api->statusBarWidgets()->setVisible(visible);
}

void WorldTab::start()
{
  Settings settings;

  setupWorldScriptWatcher();

  if (settings.getLoggingEnabled())
    openLog();

  if (!filePath.isEmpty())
  {
    try
    {
      client.loadVariables(variablesPath(filePath));
    }
    catch (const rust::Error &e)
    {
      showRustError(e);
    }
  }

  api->TextRectangle();

  restoreHistory();

  loadPlugins();

  applyWorld();

  if (settings.getAutoConnect())
    connectToHost();
}

void WorldTab::stopSound() const
{
  api->StopSound();
}

const QString &WorldTab::title() const noexcept
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

void WorldTab::onOutputBlockFormatChanged(const QTextBlockFormat &format)
{
  QTextCursor cursor(ui->output->document());
  cursor.select(QTextCursor::SelectionType::Document);
  cursor.mergeBlockFormat(format);
}

void WorldTab::onOutputFontChanged(const QFont &font)
{
  ui->output->setFont(font);
}

void WorldTab::onOutputPaddingChanged(double padding)
{
  ui->output->document()->setDocumentMargin(padding);
}

// Protected overrides

void WorldTab::closeEvent(QCloseEvent *event)
{
  OnPluginClose onPluginClose;
  api->sendCallback(onPluginClose);
  if (!promptSave())
  {
    event->ignore();
    return;
  }
  if (!filePath.isEmpty())
  {
    saveWorldAndState(filePath);
    saveHistory();
  }
  try
  {
    client.closeLog();
  }
  catch (rust::Error e)
  {
    QErrorMessage::qtHandler()->showMessage(QString::fromUtf8(e.what()));
  }
  event->accept();
}

void WorldTab::leaveEvent(QEvent *)
{
  finishDrag();
}

void WorldTab::mouseMoveEvent(QMouseEvent *)
{
  if (onDragMove) [[unlikely]]
    onDragMove->trigger();
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
  if (!resizeTimer->isActive())
    ui->output->document()->setLayoutEnabled(false);
  resizeTimer->start();
  QSplitter::resizeEvent(event);
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

void WorldTab::finishDrag()
{
  onDragMove.reset();
  if (onDragRelease) [[unlikely]]
  {
    onDragRelease->finishDrag();
    onDragRelease = nullptr;
  }
}

bool WorldTab::restoreHistory()
{
  if (filePath.isEmpty())
    return false;

  QFile file(historyPath(filePath));
  if (!file.open(QFile::ReadOnly))
    return false;

  const QByteArray history = file.readAll();

  if (file.error() != QFile::FileError::NoError || history.isEmpty())
    return false;

  QTextDocument *doc = ui->output->document();
  doc->setHtml(QString::fromUtf8(qUncompress(history)));
  autoScroll = connect(
      ui->output->verticalScrollBar(),
      &MudScrollBar::rangeChanged,
      this,
      &WorldTab::onAutoScroll);

  sessionStartBlock = doc->blockCount();

  return true;
}

bool WorldTab::saveHistory() const
{
  if (filePath.isEmpty())
    return false;

  Settings settings;

  if (!settings.getOutputHistoryEnabled())
    return false;

  const int block = ui->output->document()->blockCount();

  if (settings.getOutputHistoryLimit())
    ui->output->document()->setMaximumBlockCount(settings.getOutputHistoryLines());

  QSaveFile file(historyPath(filePath));
  if (!file.open(QSaveFile::WriteOnly))
    return false;

  if (block > sessionStartBlock + 1)
  {
    api->startLine();
    api->appendHtml(QStringLiteral("<hr/>"));
    api->startLine();
    api->startLine();
  }

  if (file.write(qCompress(ui->output->toHtml().toUtf8())) == -1)
    return false;

  return file.commit();
}

bool WorldTab::saveWorldAndState(const QString &path)
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
  setWindowModified(false);
  OnPluginSaveState onSaveState;
  api->sendCallback(onSaveState);
  try
  {
    client.saveVariables(variablesPath(path));
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
  OnPluginCommand onCommand(source, bytes);
  api->sendCallback(onCommand);
  if (onCommand.discarded())
    return true;
  OnPluginCommandEntered onCommandEntered(source, bytes);
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

void WorldTab::finishResize()
{
  ui->output->document()->setLayoutEnabled(true);
  initialized = true;
  OnPluginWorldOutputResized onWorldOutputResized;
  api->sendCallback(onWorldOutputResized);
  if (queuedConnect)
  {
    queuedConnect = false;
    connectToHost();
  }
}

void WorldTab::flushOutput()
{
  const ActionSource currentSource = api->setSource(ActionSource::TriggerFired);
  client.flush(*document);
  api->setSource(currentSource);
}

bool WorldTab::loadPlugins()
{
  const QStringList errors = client.loadPlugins();
  if (!errors.empty())
  {
    QErrorMessage::qtHandler()->showMessage(errors.join(u'\n'));
    return false;
  }
  api->initializePlugins();
  return true;
}

void WorldTab::onAutoScroll(int, int max)
{
  ui->output->verticalScrollBar()->setValue(max);
}

void WorldTab::onNewActivity()
{
  if (!alertNewActivity)
    return;
  alertNewActivity = false;
  emit newActivity(this);
  const QString sound = world.getNewActivitySound();
  if (!sound.isEmpty())
    api->PlaySound(0, sound);
}

void WorldTab::onSocketConnect()
{
  const bool isEncrypted = socket->isEncrypted();
  if (tryingSsl && !isEncrypted)
    return;

  disconnect(autoScroll);
  api->statusBarWidgets()->setConnected(
      isEncrypted ? MudStatusBar::ConnectionStatus::Encrypted
                  : MudStatusBar::ConnectionStatus::Connected);
  client.handleConnect(*socket);
  emit connectionStatusChanged(true);
  if (Settings().getDisplayConnect())
  {
    const QString format = tr("'Connected on' dddd, MMMM d, yyyy 'at' h:mm AP");
    api->appendText(QDateTime::currentDateTime().toString(format));
    api->startLine();
  }
  api->setOpen(true);
  OnPluginConnect onConnect;
  api->sendCallback(onConnect);
}

void WorldTab::onSocketDisconnect()
{
  if (tryingSsl)
    return;

  client.handleDisconnect();
  api->statusBarWidgets()->setConnected(MudStatusBar::ConnectionStatus::Disconnected);
  document->resetServerStatus();
  api->setOpen(false);
  if (Settings().getDisplayDisconnect())
  {
    const QString format = tr("'Disconnected on' dddd, MMMM d, yyyy 'at' h:mm AP");
    api->appendText(QDateTime::currentDateTime().toString(format));
    api->startLine();
  }
  OnPluginDisconnect onDisconnect;
  api->sendCallback(onDisconnect);
  emit connectionStatusChanged(false);
  if (manualDisconnect)
  {
    manualDisconnect = false;
    return;
  }
  if (Settings().getReconnectOnDisconnect())
    connectToHost();
}

void WorldTab::onSocketError(QAbstractSocket::SocketError socketError)
{
  if (socketError == QAbstractSocket::SocketError::SslHandshakeFailedError ||
      socketError == QAbstractSocket::SocketError::RemoteHostClosedError)
    return;
  api->appendText(tr("Connection error: %1").arg(socket->errorString()));
  api->startLine();
}

void WorldTab::readFromSocket()
{
  const ActionSource currentSource = api->setSource(ActionSource::TriggerFired);
  client.read(*socket, *document);
  api->setSource(currentSource);
  if (client.hasOutput())
    flushTimer->start();
  else
    flushTimer->stop();
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

class AnchorCallback : public DynamicPluginCallback
{
public:
  AnchorCallback(const QString &callback, const QString &arg)
      : DynamicPluginCallback(callback),
        arg(arg) {}

  inline constexpr ActionSource source() const noexcept override { return ActionSource::UserMenuAction; }

  int pushArguments(lua_State *L) const override
  {
    qlua::pushQString(L, arg);
    return 1;
  }

private:
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

  int delimIndex = 0;
  int fnIndex = 0;
  if (
      action.first(2) == QStringLiteral("!!") &&
      action.back() == u')' &&
      (delimIndex = action.indexOf(u':')) != -1 &&
      (fnIndex = action.indexOf(u'(', delimIndex)) != -1)
  {
    const QString pluginID = action.sliced(2, delimIndex - 2);
    const QString functionName = action.sliced(delimIndex + 1, fnIndex - delimIndex - 1);
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
