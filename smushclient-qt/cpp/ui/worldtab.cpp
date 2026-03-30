#include "worldtab.h"
#include "../bridge/document.h"
#include "../components/mudscrollbar.h"
#include "../environment.h"
#include "../hotkeys.h"
#include "../localization.h"
#include "../mudstatusbar/mudstatusbar.h"
#include "../scripting/callback/plugincallback.h"
#include "../scripting/miniwindow/hotspot.h"
#include "../scripting/qlua.h"
#include "../scripting/scriptapi.h"
#include "../settings.h"
#include "../spans.h"
#include "dialog/saveprompt.h"
#include "dialog/styledialog.h"
#include "smushclient_qt/src/ffi/sender.cxxqt.h"
#include "smushclient_qt/src/ffi/world.cxxqt.h"
#include "ui_worldtab.h"
#include "worlddetails/regexdialog.h"
#include "worlddetails/worlddetails.h"
#include <QtCore/QSaveFile>
#include <QtCore/QTimer>
#include <QtCore/QUrl>
#include <QtGui/QAbstractTextDocumentLayout>
#include <QtGui/QAction>
#include <QtGui/QDesktopServices>
#include <QtGui/QFontDatabase>
#include <QtGui/QPalette>
#include <QtNetwork/QNetworkProxy>
#include <QtWidgets/QErrorMessage>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>
#include <string>

using std::nullopt;
using std::string;
using std::string_view;
using std::chrono::milliseconds;

constexpr const Qt::KeyboardModifiers numpadMods =
  Qt::KeyboardModifier::ControlModifier | Qt::KeyboardModifier::MetaModifier;

// Private utilities

namespace {
QString
historyPath(const QString& path)
{
  return path + QStringLiteral(".history");
}

inline void
showRustError(const rust::Error& e)
{
  QErrorMessage::qtHandler()->showMessage(QString::fromUtf8(e.what()));
}
} // namespace

// Public methods

WorldTab::WorldTab(Notepads& notepads, QWidget* parent)
  : QSplitter(parent)
  , ui(new Ui::WorldTab)
  , actionTextAttributes(
      new QAction(QIcon::fromTheme(QIcon::ThemeIcon::HelpFaq),
                  tr("Text Attributes"),
                  this))
  , flushTimer(new QTimer(this))
  , resizeTimer(new QTimer(this))
  , scriptReloadOption(ScriptRecompile::Never)
#ifdef QT_NO_SSL
  , socket(new QTcpSocket(this))
#else
  , socket(new QSslSocket(this))
#endif
  , worldScriptWatcher(this)
{
  ui->setupUi(this);
  // NOLINTBEGIN(cppcoreguidelines-prefer-member-initializer,
  // cppcoreguidelines-prefer-member-initializer)
  api = new ScriptApi(client, *socket, *ui->output, notepads, *this);
  document = new Document(*ui->output, *api, this);
  // NOLINTEND(cppcoreguidelines-prefer-member-initializer,
  // cppcoreguidelines-prefer-member-initializer)

#ifndef QT_NO_SSL
  connect(socket, &QSslSocket::readyRead, this, &WorldTab::readFromSocket);
  connect(socket, &QSslSocket::connected, this, &WorldTab::onSocketConnect);
  connect(
    socket, &QSslSocket::disconnected, this, &WorldTab::onSocketDisconnect);
  connect(socket, &QSslSocket::encrypted, this, &WorldTab::onSocketConnect);
  connect(socket, &QSslSocket::errorOccurred, this, &WorldTab::onSocketError);
#endif

  flushTimer->setInterval(milliseconds{ 2000 });
  flushTimer->setSingleShot(true);
  connect(flushTimer, &QTimer::timeout, this, &WorldTab::flushOutput);

  resizeTimer->setInterval(milliseconds{ 100 });
  resizeTimer->setSingleShot(true);
  connect(resizeTimer, &QTimer::timeout, this, &WorldTab::finishResize);
  connect(ui->output,
          &MudBrowser::aliasMenuRequested,
          this,
          &WorldTab::onAliasMenuRequested);
  connect(document, &Document::newActivity, this, &WorldTab::onNewActivity);
  connect(&worldScriptWatcher,
          &QFileSystemWatcher::fileChanged,
          this,
          &WorldTab::confirmReloadWorldScript);

  ui->input->setFocus();

  const Settings settings;
  ui->input->setPalette(settings.getInputPalette());
  ui->input->setFont(settings.getInputFont());
  ui->output->setFont(settings.getOutputFont());

  const QTextEdit::LineWrapMode wrapMode =
    settings.getOutputWrapping() ? QTextEdit::LineWrapMode::WidgetWidth
                                 : QTextEdit::LineWrapMode::NoWrap;
  ui->output->setLineWrapMode(wrapMode);

  QTextDocument& doc = *ui->output->document();
  doc.setDocumentMargin(settings.getOutputPadding());

  QTextCursor formatCursor(&doc);
  formatCursor.select(QTextCursor::SelectionType::Document);
  formatCursor.mergeBlockFormat(settings.getOutputBlockFormat());
}

WorldTab::~WorldTab()
{
  disconnect(socket, nullptr, nullptr, nullptr);
  delete api;
  delete ui;
}

WorldTab::AvailableCopy
WorldTab::availableCopy() const noexcept
{
  if (outputCopyAvailable) {
    return AvailableCopy::Output;
  }
  if (inputCopyAvailable) {
    return AvailableCopy::Input;
  }
  return AvailableCopy::None;
}

void
WorldTab::clearCallbacks()
{
  onDragMove = nullopt;
  onDragRelease = nullptr;
}

void
WorldTab::clearCallbacks(const Plugin& plugin)
{
  if (onDragMove && onDragMove->belongsToPlugin(plugin)) {
    onDragMove = nullopt;
  }
  if (onDragRelease != nullptr && onDragRelease->belongsToPlugin(plugin)) {
    onDragRelease = nullptr;
  }
}

void
WorldTab::closeLog()
{
  try {
    client.tryCloseLog();
  } catch (const rust::Error& e) {
    showRustError(e);
  }
}

QTextEdit*
WorldTab::copyableEditor() const
{
  if (outputCopyAvailable) {
    return ui->output;
  }
  if (inputCopyAvailable) {
    return ui->input;
  }
  return nullptr;
}

bool
WorldTab::connectToHost()
{
  if (!initialized) {
    queuedConnect = true;
    return true;
  }

  if (socket->state() != QAbstractSocket::SocketState::UnconnectedState) {
    return false;
  }

  client.connectToHost(*socket);
  return true;
}

bool
WorldTab::disconnectFromHost()
{
  if (socket->state() == QAbstractSocket::SocketState::UnconnectedState) {
    return false;
  }
  manualDisconnect = true;
  socket->disconnectFromHost();
  return true;
}

void
WorldTab::editWorldScript()
{
  if (!QDesktopServices::openUrl(QUrl::fromLocalFile(worldScriptPath))) {
    QErrorMessage::qtHandler()->showMessage(
      tr("Failed to open file: %1").arg(worldScriptPath));
  }
}

bool
WorldTab::importWorld(const QString& filename) &
{
  try {
    const RegexParse result = client.tryImportWorld(filename);
    if (!result.success) {
      RegexDialog dialog(result, this);
      dialog.exec();
      return false;
    }
  } catch (const rust::Error& e) {
    showRustError(e);
    return false;
  }
  ui->output->setVerticalScrollBarPolicy(
    Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
  return true;
}

void
WorldTab::initNew() &
{
  worldName = tr("New world");
}

bool
WorldTab::isConnected() const
{
  return socket->state() != QAbstractSocket::SocketState::UnconnectedState;
}

void
WorldTab::openLog()
{
  try {
    client.tryOpenLog();
  } catch (const rust::Error& e) {
    showRustError(e);
  }
}

bool
WorldTab::openWorld(const QString& filename) &
{
  try {
    client.tryLoadWorld(filename);
  } catch (const rust::Error& e) {
    showRustError(e);
    return false;
  }
  filePath = filename;
  ui->output->setVerticalScrollBarPolicy(
    Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
  return true;
}

bool
WorldTab::openWorldSettings()
{
  World world(client);
  const QString logFileName = world.getAutoLogFileName();
  WorldPrefs worlddetails(world, client, *api, this);
  if (worlddetails.exec() != QDialog::Accepted) {
    return false;
  }
  if (worlddetails.isDirty()) {
    setWindowModified(true);
  }

  if (!client.setWorld(world)) {
    return false;
  }

  if (!world.getSaveWorldAutomatically()) {
    setWindowModified(true);
  }

  applyWorld(world);

  if (world.getAutoLogFileName() != logFileName && client.isLogOpen()) {
    client.closeLog();
    openLog();
  }

  return true;
}

bool
WorldTab::promptSave()
{
  if (!isWindowModified()) {
    return true;
  }
  switch (Settings().getWorldCloseBehavior()) {
    case Settings::WorldCloseBehavior::Save:
      saveWorld();
      return true;
    case Settings::WorldCloseBehavior::Discard:
      return true;
    case Settings::WorldCloseBehavior::Confirm:
      break;
  }
  SavePrompt prompt(worldName, this);
  switch (prompt.exec()) {
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

void
WorldTab::reloadWorldScript() const
{
  api->reloadWorldScript(worldScriptPath);
}

void
WorldTab::resetAllTimers() const
{
  api->resetAllTimers();
}

QString
WorldTab::saveWorld()
{
  if (filePath.isEmpty()) {
    return saveWorldAsNew(QString());
  }
  if (!saveWorldAndState(filePath)) {
    return QString();
  }
  return filePath;
}

QString
WorldTab::saveWorldAsNew(const QString& path, bool separate)
{
  const QString userPath =
    path.isEmpty()
      ? QFileDialog::getSaveFileName(this,
                                     tr("Save as"),
                                     Settings().getWorldsDir() +
                                       QDir::separator() + worldName,
                                     FileFilter::world())
      : path;

  if (userPath.isEmpty() || !saveWorldAndState(userPath)) {
    return QString();
  }

  if (!separate) {
    filePath = makePathRelative(path);
  }

  return filePath;
}

bool
WorldTab::sendCommand(const QString& command, CommandSource source)
{
  if (speedWalkPrefix != u'\0' && command.startsWith(speedWalkPrefix)) {
    try {
      api->enqueueCommand(client.tryEvaluateSpeedwalk(command.sliced(1)), true);
    } catch (const rust::Error& e) {
      ; // just treat it like a normal command
    }
  }

  const AliasOutcomes aliasOutcome = client.alias(command, source, *document);

  if (aliasOutcome.testFlag(AliasOutcome::Remember)) {
    ui->input->remember(command);
  }

  if (!aliasOutcome.testFlag(AliasOutcome::Send)) {
    return true;
  }

  SendFlags flags = SendFlag::Log;
  if (aliasOutcome.testFlag(AliasOutcome::Echo)) {
    flags |= SendFlag::Echo;
  }

  const ActionSource actionSource = source >= CommandSource::Hotkey
                                      ? ActionSource::UserTyping
                                      : ActionSource::UserKeypad;

  QByteArray bytes = command.toUtf8();
  OnPluginCommand onCommand(actionSource, bytes);
  api->sendCallback(onCommand);
  if (onCommand.discarded()) {
    return true;
  }
  OnPluginCommandEntered onCommandEntered(actionSource, bytes);
  api->sendCallback(onCommandEntered);
  if (bytes.size() == 1) {
    switch (bytes.front()) {
      case '\t':
        return true;
      case '\r':
        return false;
      default:
        break;
    }
  }
  api->sendToWorld(bytes, command, flags);
  return true;
}

const QHash<QString, QString>&
WorldTab::serverStatus() const
{
  return document->serverStatus();
}

void
WorldTab::setActive(bool active)
{
  m_active = active;
  alertNewActivity = !active;
  if (!active) {
    OnPluginLoseFocus onLoseFocus;
    api->sendCallback(onLoseFocus);
    return;
  }
  ui->input->focusWidget();
  OnPluginGetFocus onGetFocus;
  api->sendCallback(onGetFocus);
}

void
WorldTab::setOnDragMove(const Plugin& plugin,
                        const PluginCallback& callback,
                        QObject* parent)
{
  CallbackTrigger trigger(plugin, callback, parent);
  if (!trigger.valid()) {
    return;
  }
  onDragMove.emplace(std::move(trigger));
}

void
WorldTab::setOnDragRelease(Hotspot* hotspot)
{
  onDragRelease = hotspot;
}

void
WorldTab::setStatusBarVisible(bool visible)
{
  api->statusBar()->setVisible(visible);
}

void
WorldTab::setTitle(const QString& title)
{
  if (m_title == title) {
    return;
  }
  m_title = title;
  emit titleChanged(this, this->title());
}

ApiCode
WorldTab::setWorldOption(size_t pluginIndex, string_view name, int64_t value)
{
  const ApiCode result = client.setWorldOption(pluginIndex, name, value);
  if (result != ApiCode::OK) {
    return result;
  }

  if (name == "enable_command_stack") {
    splitOn = client.commandSplitter();
  } else if (name == "enable_speed_walk") {
    speedWalkPrefix = client.speedWalkPrefix();
  } else if (name == "keypad_enable") {
    handleKeypad = value == 1;
    ui->input->setKeypadIgnored(handleKeypad);
    ui->output->setKeypadIgnored(handleKeypad);
  } else if (name == "script_reload_option") {
    scriptReloadOption = static_cast<ScriptRecompile>(value);
  }

  return result;
}

ApiCode
WorldTab::setWorldAlphaOption(size_t pluginIndex,
                              string_view name,
                              string_view value)
{
  const ApiCode result = client.setWorldAlphaOption(pluginIndex, name, value);
  if (result != ApiCode::OK) {
    return result;
  }

  if (name == "command_stack_character") {
    splitOn = client.commandSplitter();
  } else if (name == "name") {
    worldName = QString::fromUtf8(value);
    if (m_title.isEmpty()) {
      emit titleChanged(this, worldName);
    }
  } else if (name == "script_path") {
    worldScriptPath = QString::fromUtf8(value);
  } else if (name == "speed_walk_prefix") {
    speedWalkPrefix = client.speedWalkPrefix();
  }

  return result;
}

void
WorldTab::simulateOutput(string_view output) const noexcept
{
  client.simulate(output, *document);
}

void
WorldTab::start()
{
  Settings settings;

  if (settings.getLoggingEnabled()) {
    openLog();
  }

  if (!filePath.isEmpty()) {
    try {
      client.tryLoadVariables(variablesPath());
    } catch (const rust::Error& e) {
      showRustError(e);
    }
  }

  api->TextRectangle();

  restoreHistory();

  applyWorld(World(client));

  loadPlugins();

  setupWorldScriptWatcher();

  if (settings.getAutoConnect()) {
    connectToHost();
  }
}

MudStatusBar*
WorldTab::statusBar() const
{
  return api->statusBar();
}

void
WorldTab::stopSound() const
{
  api->StopSound();
}

// Public slots

void
WorldTab::onInputBackgroundChanged(const QColor& color) const
{
  QPalette palette = ui->input->palette();
  palette.setColor(QPalette::ColorRole::Base, color);
  ui->input->setPalette(palette);
}

void
WorldTab::onInputFontChanged(const QFont& font) const
{
  ui->input->setFont(font);
}

void
WorldTab::onInputForegroundChanged(const QColor& color) const
{
  QPalette palette = ui->input->palette();
  palette.setColor(QPalette::ColorRole::Text, color);
  ui->input->setPalette(palette);
}

void
WorldTab::onOutputBlockFormatChanged(const QTextBlockFormat& format) const
{
  QTextCursor cursor(ui->output->document());
  cursor.select(QTextCursor::SelectionType::Document);
  cursor.mergeBlockFormat(format);
}

void
WorldTab::onOutputFontChanged(const QFont& font) const
{
  ui->output->setFont(font);
}

void
WorldTab::onOutputPaddingChanged(qreal padding) const
{
  ui->output->document()->setDocumentMargin(padding);
}

// Protected overrides

void
WorldTab::closeEvent(QCloseEvent* event)
{
  OnPluginClose onClose;
  api->sendCallback(onClose);
  if (!promptSave()) {
    event->ignore();
    return;
  }
  if (!filePath.isEmpty()) {
    saveWorldAndState(filePath);
    saveHistory();
  }
  event->accept();
}

void
WorldTab::leaveEvent(QEvent* /*event*/)
{
  finishDrag();
}

void
WorldTab::mouseMoveEvent(QMouseEvent* /*event*/)
{
  if (onDragMove) [[unlikely]] {
    onDragMove->trigger();
  }
}

void
WorldTab::keyPressEvent(QKeyEvent* event)
{
  const Qt::KeyboardModifiers modifiers = event->modifiers();
  if (!handleKeypad ||
      !modifiers.testFlag(Qt::KeyboardModifier::KeypadModifier)) [[likely]] {
    QSplitter::keyPressEvent(event);
    return;
  }
  QString action =
    hotkeys.get(Qt::Key(event->key()), modifiers.testAnyFlags(numpadMods));
  if (action.isEmpty()) {
    return;
  }
  sendCommand(action, CommandSource::Hotkey);
}

void
WorldTab::mouseReleaseEvent(QMouseEvent* /*event*/)
{
  finishDrag();
}

void
WorldTab::resizeEvent(QResizeEvent* event)
{
  api->onResize(false);
  if (!resizeTimer->isActive()) {
    ui->output->document()->setLayoutEnabled(false);
  }
  resizeTimer->start();
  QSplitter::resizeEvent(event);
}

// Private methods

void
WorldTab::applyWorld(const World& world)
{
  commandStackDelay = world.getCommandStackDelay();
  scriptReloadOption = world.getScriptReloadOption();
  worldName = world.getName();
  if (m_title.isEmpty()) {
    emit titleChanged(this, worldName);
  }
  worldScriptPath = world.getWorldScript();
  handleKeypad = world.getKeypadEnable();
  ui->input->setKeypadIgnored(handleKeypad);
  ui->output->setKeypadIgnored(handleKeypad);
  QPalette palette;
  const QColor fgColor = world.getAnsi7();
  const QColor bgColor = world.getAnsi0();
  palette.setColor(QPalette::Text, fgColor);
  palette.setColor(QPalette::HighlightedText, fgColor);
  palette.setColor(QPalette::Base, bgColor);
  palette.setColor(QPalette::AlternateBase, bgColor);
  ui->background->setPalette(palette);
  if (world.getSaveWorldAutomatically()) {
    saveWorldAndState(filePath);
  }

  hotkeys.applyWorld(world);
  api->applyWorld(world);
  ui->output->cursor()->applyWorld(world);
  updateWorldScript();
  speedWalkPrefix = client.speedWalkPrefix();
  splitOn = client.commandSplitter();
}

void
WorldTab::finishDrag()
{
  onDragMove.reset();
  if (onDragRelease) [[unlikely]] {
    onDragRelease->finishDrag();
    onDragRelease = nullptr;
  }
}

void
WorldTab::handleConnect()
{
  client.handleConnect(*socket);
  if (Settings().getDisplayConnect()) {
    const QString format = tr("'Connected on' dddd, MMMM d, yyyy 'at' h:mm AP");
    MudCursor* cursor = ui->output->cursor();
    cursor->appendText(QDateTime::currentDateTime().toString(format));
    cursor->startLine();
  }
  api->setOpen(true);
  OnPluginConnect onConnect;
  api->sendCallback(onConnect);
}

bool
WorldTab::restoreHistory()
{
  if (filePath.isEmpty()) {
    return false;
  }

  QFile file(historyPath(filePath));
  if (!file.open(QFile::ReadOnly)) {
    return false;
  }

  const QByteArray history = file.readAll();

  if (file.error() != QFile::FileError::NoError || history.isEmpty()) {
    return false;
  }

  QTextDocument& doc = *ui->output->document();
  doc.setHtml(QString::fromUtf8(qUncompress(history)));
  autoScroll = connect(ui->output->verticalScrollBar(),
                       &MudScrollBar::rangeChanged,
                       this,
                       &WorldTab::onAutoScroll);

  sessionStartBlock = doc.blockCount();

  return true;
}

bool
WorldTab::saveHistory() const
{
  if (filePath.isEmpty()) {
    return false;
  }

  Settings settings;

  if (!settings.getOutputHistoryEnabled()) {
    return false;
  }

  const int block = ui->output->document()->blockCount();

  if (settings.getOutputHistoryLimit()) {
    ui->output->document()->setMaximumBlockCount(
      settings.getOutputHistoryLines());
  }

  QSaveFile file(historyPath(filePath));
  if (!file.open(QSaveFile::WriteOnly)) {
    return false;
  }

  if (block > sessionStartBlock + 1) {
    MudCursor* cursor = ui->output->cursor();
    cursor->startLine();
    cursor->appendHtml(QStringLiteral("<hr/>"));
    cursor->startLine();
    cursor->startLine();
  }

  if (file.write(qCompress(ui->output->toHtml().toUtf8())) == -1) {
    return false;
  }

  return file.commit();
}

bool
WorldTab::saveWorldAndState(const QString& path)
{
  OnPluginWorldSave onWorldSave;
  api->sendCallback(onWorldSave);
  try {
    client.trySaveWorld(path);
  } catch (const rust::Error& e) {
    showRustError(e);
    return false;
  }
  setWindowModified(false);
  OnPluginSaveState onSaveState;
  api->sendCallback(onSaveState);
  try {
    client.trySaveVariables(variablesPath(path));
  } catch (const rust::Error& e) {
    showRustError(e);
  }
  return true;
}

void
WorldTab::setupWorldScriptWatcher()
{
  const QStringList watchedScripts = worldScriptWatcher.files();
  if (!watchedScripts.isEmpty()) {
    worldScriptWatcher.removePaths(watchedScripts);
  }
  if (!worldScriptPath.isEmpty()) {
    worldScriptWatcher.addPath(worldScriptPath);
  }
}

void
WorldTab::showAliasMenu()
{
  const rust::Vec<AliasMenuItem> items = client.aliasMenu();
  if (items.empty()) {
    return;
  }
  QMenu menu(this);
  for (const AliasMenuItem& item : items) {
    QAction* action = new QAction(&menu);
    action->setText(item.text);
    action->setData(QPoint(static_cast<int>(item.plugin), item.id));
    menu.addAction(action);
  }
  const QAction* choice = menu.exec(QCursor::pos());
  if (choice == nullptr) {
    return;
  }
  const QPoint data = choice->data().toPoint();
  client.invokeAlias(data.x(), data.y(), *document);
}

QStringList
WorldTab::splitCommands(const QString& input) const
{
  constexpr const QChar delim = static_cast<uint16_t>(31);
  if (splitOn == u'\n') {
    return input.split(u'\n');
  }
  if (input.startsWith(splitOn)) {
    QStringList commands = input.split(u'\n');
    commands.first().removeFirst();
    return commands;
  }
  QString buf = input;
  const std::array<QChar, 2> doubled = { splitOn, splitOn };
  buf.replace(doubled.data(), doubled.size(), &delim, 1);
  buf.replace(splitOn, u'\n');
  buf.replace(delim, splitOn);
  return buf.split(u'\n');
}

void
WorldTab::updateWorldScript()
{
  const QStringList watchedScripts = worldScriptWatcher.files();
  if (watchedScripts.value(0) == worldScriptPath) {
    return;
  }
  if (!watchedScripts.isEmpty()) {
    worldScriptWatcher.removePaths(watchedScripts);
  }
  if (!worldScriptPath.isEmpty()) {
    worldScriptWatcher.addPath(worldScriptPath);
  }
  confirmReloadWorldScript(worldScriptPath);
}

// Private slots

void
WorldTab::confirmReloadWorldScript(const QString& worldScriptPath)
{
  QFileInfo info(worldScriptPath);
  if (!info.isFile() || !info.isReadable()) {
    return;
  }
  switch (scriptReloadOption) {
    case ScriptRecompile::Always:
      break;
    case ScriptRecompile::Never:
      return;
    case ScriptRecompile::Confirm:
      if (QMessageBox::question(
            this,
            tr("World script changed"),
            tr("Would you like to reload the world script?")) !=
          QMessageBox::StandardButton::Yes) {
        return;
      }
  }
  api->reloadWorldScript(worldScriptPath);
}

void
WorldTab::finishResize()
{
  ui->output->document()->setLayoutEnabled(true);
  initialized = true;
  OnPluginWorldOutputResized onWorldOutputResized;
  api->onResize(true);
  api->sendCallback(onWorldOutputResized);
  if (queuedConnect) {
    queuedConnect = false;
    connectToHost();
  }
}

void
WorldTab::flushOutput()
{
  const ActionSource currentSource = api->setSource(ActionSource::TriggerFired);
  client.flush(*document);
  api->setSource(currentSource);
  api->sendPartialLineToPlugins();
}

void
WorldTab::loadPlugins()
{
  const QStringList errors = client.loadPlugins();
  if (!errors.empty()) {
    QErrorMessage::qtHandler()->showMessage(errors.join(u'\n'));
  }
  api->initializePlugins();
}

void
WorldTab::onAliasMenuRequested(const QString& word)
{
  api->setWordUnderMenu(word);
  showAliasMenu();
}

void
WorldTab::onAutoScroll(int /*min*/, int max) const
{
  ui->output->verticalScrollBar()->setValue(max);
}

void
WorldTab::onNewActivity()
{
  if (!alertNewActivity) {
    return;
  }
  alertNewActivity = false;
  emit newActivity(this);
  client.handleAlert();
}

void
WorldTab::onSocketConnect()
{
  disconnect(autoScroll);
#ifdef QT_NO_SSL
  api->statusBarWidgets()->setConnected(
    MudStatusBar::ConnectionStatus::Connected);
  emit connectionStatusChanged(true);
  handleConnect();
#else
  api->statusBar()->setConnected(socket->isEncrypted()
                                   ? MudStatusBar::ConnectionStatus::Encrypted
                                   : MudStatusBar::ConnectionStatus::Connected);
  emit connectionStatusChanged(true);
  if (socket->mode() == QSslSocket::UnencryptedMode) {
    handleConnect();
  }
#endif
}

void
WorldTab::onSocketDisconnect()
{
  client.handleDisconnect();
  api->statusBar()->setConnected(MudStatusBar::ConnectionStatus::Disconnected);
  document->resetServerStatus();
  api->setOpen(false);
  if (Settings().getDisplayDisconnect()) {
    const QString format =
      tr("'Disconnected on' dddd, MMMM d, yyyy 'at' h:mm AP");
    MudCursor* cursor = ui->output->cursor();
    cursor->appendText(QDateTime::currentDateTime().toString(format));
    cursor->startLine();
  }
  OnPluginDisconnect onDisconnect;
  api->sendCallback(onDisconnect);
  emit connectionStatusChanged(false);
  if (manualDisconnect) {
    manualDisconnect = false;
    return;
  }
  if (Settings().getReconnectOnDisconnect()) {
    connectToHost();
  }
}

#ifndef QT_NO_SSL
void
WorldTab::onSocketEncrypted()
{
  api->statusBar()->setConnected(MudStatusBar::ConnectionStatus::Encrypted);
  handleConnect();
}
#endif

void
WorldTab::onSocketError(QAbstractSocket::SocketError socketError)
{
  if (socketError == QAbstractSocket::SocketError::SslHandshakeFailedError ||
      socketError == QAbstractSocket::SocketError::RemoteHostClosedError) {
    return;
  }
  MudCursor* cursor = ui->output->cursor();
  cursor->appendText(tr("Connection error: %1").arg(socket->errorString()));
  cursor->startLine();
}

void
WorldTab::readFromSocket()
{
  const ActionSource currentSource = api->setSource(ActionSource::TriggerFired);
  client.read(*socket, *document);
  api->setSource(currentSource);
  if (client.hasOutput()) {
    flushTimer->start();
  } else {
    flushTimer->stop();
  }
}

void
WorldTab::on_input_copyAvailable(bool available)
{
  inputCopyAvailable = available;
  emit copyAvailable(availableCopy());
}

void
WorldTab::on_input_submitted(const QString& input)
{
  ui->output->verticalScrollBar()->setPaused(false);

  const QStringList commands = splitCommands(input);

  bool eraseInput = commands.length() > 1;

  bool startQueue = commandStackDelay && eraseInput && splitOn != u'\n' &&
                    input.contains(splitOn);

  for (const QString& command : commands) {
    eraseInput = sendCommand(command, CommandSource::User) || eraseInput;
    if (startQueue) {
      api->startCommandQueueTimer();
      startQueue = false;
    }
  }

  if (eraseInput) {
    ui->input->clear();
  }
}

void
WorldTab::on_input_textChanged()
{
  OnPluginCommandChanged onCommandChanged;
  api->sendCallback(onCommandChanged);
}

class AnchorCallback : public DynamicPluginCallback
{
public:
  AnchorCallback(const QString& callback, const QString& arg) noexcept
    : DynamicPluginCallback(callback)
    , arg(arg)
  {
  }

  constexpr ActionSource source() const noexcept override
  {
    return ActionSource::UserMenuAction;
  }

  int pushArguments(lua_State* L) const override
  {
    qlua::push(L, arg);
    return 1;
  }

private:
  const QString& arg;
};

void
WorldTab::on_output_copyAvailable(bool available)
{
  outputCopyAvailable = available;
  emit copyAvailable(availableCopy());
}

void
WorldTab::on_output_customContextMenuRequested(const QPoint& pos)
{
  const QPoint mouse = ui->output->mapToGlobal(pos);
  QMenu* menu = ui->output->createStandardContextMenu(mouse);
  menu->addAction(actionTextAttributes);
  if (menu->exec(mouse) != actionTextAttributes) {
    return;
  }
  const QTextCharFormat format = ui->output->formatAt(pos);
  StyleDialog dialog(format, this);
  dialog.exec();
}

void
WorldTab::on_output_linkActivated(const QString& action, SendTo sendTo)
{
  switch (sendTo) {
    case SendTo::Internet:
      QDesktopServices::openUrl(QUrl(action));
      return;
    case SendTo::World:
      break;
    case SendTo::Prompt:
      ui->input->setText(action);
      return;
  }

  qsizetype delimIndex = 0;
  qsizetype fnIndex = 0;
  if (action.first(2) == QStringLiteral("!!") && action.back() == u')' &&
      (delimIndex = action.indexOf(u':')) != -1 &&
      (fnIndex = action.indexOf(u'(', delimIndex)) != -1) {
    const QString pluginID = action.sliced(2, delimIndex - 2);
    const QString functionName =
      action.sliced(delimIndex + 1, fnIndex - delimIndex - 1);
    const QString arg = action.sliced(fnIndex + 1, action.size() - fnIndex - 2);
    AnchorCallback callback(functionName, arg);
    api->sendCallback(callback, pluginID);
    return;
  }

  sendCommand(action, CommandSource::Hotkey);
}

void
WorldTab::on_output_linkMenuActivated(const QPoint& pos,
                                      const QString& commands,
                                      const QString& labels,
                                      SendTo sendTo)
{
  const QTextCharFormat format =
    ui->output->cursorForPosition(pos).charFormat();
  const QPoint mouse = ui->output->mapToGlobal(pos);
  const QStringList commandList = commands.split(u'|');
  const QStringList labelList = labels.split(u'|');
  QMenu menu(ui->output);
  auto label = labelList.cbegin();
  auto labelEnd = labelList.cend();
  for (const QString& command : commandList) {
    QAction* action =
      label == labelEnd ? menu.addAction(command) : menu.addAction(*label++);
    action->setData(command);
  }

  const QAction* chosen = menu.exec(mouse);
  if (chosen == nullptr) {
    return;
  }
  const QString choice = chosen->data().toString();
  if (sendTo == SendTo::Prompt) {
    ui->input->setText(choice);
    return;
  }
  const ActionSource oldSource = api->setSource(ActionSource::UserMenuAction);
  api->sendToWorld(choice,
                   SendFlag::Echo | SendFlag::Log | SendFlag::Immediate);
  api->setSource(oldSource);
}

void
WorldTab::on_output_selectionChanged()
{
  OnPluginSelectionChanged onSelectionChanged;
  api->sendCallback(onSelectionChanged);
}
