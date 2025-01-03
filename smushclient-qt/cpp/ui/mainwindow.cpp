#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtCore/QSaveFile>
#include <QtGui/QClipboard>
#include <QtGui/QDesktopServices>
#include <QtGui/QScreen>
#include <QtGui/QTextBlock>
#include <QtGui/QTextDocumentFragment>
#include <QtPrintSupport/QPrintDialog>
#include <QtPrintSupport/QPrinter>
#include <QtWidgets/QErrorMessage>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QStatusBar>
#include "ui_worldtab.h"
#include "finddialog.h"
#include "notepad.h"
#include "serverstatus.h"
#include "worldtab.h"
#include "settings/settings.h"
#include "../environment.h"
#include "../mudstatusbar/mudstatusbar.h"
#include "../settings.h"
#include "../spans.h"
#include "../components/mudscrollbar.h"
#include "../localization.h"
#include "../scripting/listbox.h"

MainWindow::MainWindow(Notepads *notepads, QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      notepads(notepads)
{
  setAttribute(Qt::WA_DeleteOnClose);
  ui->setupUi(this);

  findDialog = new FindDialog(this);
  QDir::setCurrent(settings.getStartupDirectoryOrDefault());

  ui->action_status_bar->setChecked(settings.getShowStatusBar());
  ui->action_wrap_output->setChecked(settings.getOutputWrapping());
  connect(ui->action_close_window, &QAction::triggered, this, &MainWindow::close);
  connect(ui->action_log_session, &QAction::triggered, &settings, &Settings::setLoggingEnabled);
  connect(ui->action_maximize, &QAction::triggered, this, &MainWindow::showMaximized);
  connect(ui->action_minimize, &QAction::triggered, this, &MainWindow::showMinimized);
  connect(ui->action_restore, &QAction::triggered, this, &MainWindow::showNormal);
  connect(ui->action_status_bar, &QAction::triggered, &settings, &Settings::setShowStatusBar);
  connect(ui->action_wrap_output, &QAction::triggered, &settings, &Settings::setOutputWrapping);

  recentFileActions = QList<QAction *>{
      ui->action_rec_1,
      ui->action_rec_2,
      ui->action_rec_3,
      ui->action_rec_4,
      ui->action_rec_5};

  ui->action_log_session->setChecked(settings.getLoggingEnabled());

  setupRecentFiles(settings.getRecentFiles());

  restore();
}

MainWindow::~MainWindow()
{
  save();
  delete ui;
}

void MainWindow::openWorld(const QString &filePath)
{
  WorldTab *tab = createWorldTab(ui->world_tabs);
  if (tab->openWorld(filePath))
  {
    connectTab(tab);
    const int tabIndex = ui->world_tabs->addTab(tab, tab->title());
    ui->world_tabs->setCurrentIndex(tabIndex);
    addRecentFile(filePath);
    tab->start();
    return;
  }
  delete tab;
  const RecentFileResult result = settings.removeRecentFile(filePath);
  if (result.changed)
    setupRecentFiles(result.recentFiles);
}

// Protected methods

void MainWindow::closeEvent(QCloseEvent *event)
{
  QSettings().setValue(settingsKey(), saveGeometry());

  if (settings.getConfirmQuit() && QMessageBox::question(this, QString(), tr("Close all worlds and quit?"), QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Cancel)
  {
    event->ignore();
    return;
  }

  QList<WorldTab *> tabs;
  const int tabCount = ui->world_tabs->count();
  tabs.reserve(tabCount);
  QStringList lastFiles;
  lastFiles.reserve(tabCount);

  for (int i = 0; i < tabCount; ++i)
  {
    WorldTab *tab = qobject_cast<WorldTab *>(ui->world_tabs->widget(i));
    if (!tab->promptSave())
    {
      event->ignore();
      return;
    }
    const QString &worldFilePath = tab->worldFilePath();
    if (!worldFilePath.isEmpty())
      lastFiles.push_back(worldFilePath);
    tabs.push_back(tab);
  }

  for (WorldTab *tab : tabs)
  {
    tab->setWindowModified(false);
    tab->close();
  }

  settings.setLastFiles(lastFiles);
  event->accept();
}

bool MainWindow::event(QEvent *event)
{

  switch (event->type())
  {
  case QEvent::WindowActivate:
    if (WorldTab *tab = worldtab(); tab)
      tab->setIsActive(true);
    break;

  case QEvent::WindowDeactivate:
    if (WorldTab *tab = worldtab(); tab)
      tab->setIsActive(false);
    break;

  default:
    break;
  };

  return QMainWindow::event(event);
}

// Private static methods

const QString &MainWindow::settingsKey()
{
  static const QString key = QStringLiteral("state/mainwindow");
  return key;
}

// Private methods

void MainWindow::addRecentFile(const QString &filePath)
{
  if (filePath.isEmpty())
    return;

  const RecentFileResult result = settings.addRecentFile(filePath);
  if (!result.changed)
    return;

  setupRecentFiles(result.recentFiles);
}

WorldTab *MainWindow::createWorldTab(QWidget *parent) const
{
  MudStatusBar *mudStatusBar = new MudStatusBar;
  statusBar()->addPermanentWidget(mudStatusBar);
  mudStatusBar->hide();
  WorldTab *tab = new WorldTab(mudStatusBar, notepads, parent);
  tab->setAttribute(Qt::WA_DeleteOnClose);
  return tab;
}

void MainWindow::connectTab(WorldTab *tab) const
{
  connect(tab, &WorldTab::copyAvailable, this, &MainWindow::onCopyAvailable);
  connect(tab, &WorldTab::newActivity, this, &MainWindow::onNewActivity);
  SettingsDialog::connect(tab);
}

void MainWindow::openRecentFile(qsizetype index)
{
  const QStringList recentFiles = settings.getRecentFiles();
  if (index >= recentFiles.length())
    return;

  const QString &filePath = recentFiles.at(index);

  for (int i = 0, end = ui->world_tabs->count(); i < end; ++i)
  {
    WorldTab *tab = worldtab(i);
    if (tab->worldFilePath() == filePath)
    {
      ui->world_tabs->setCurrentWidget(tab);
      return;
    }
  }

  openWorld(recentFiles.at(index));
}

bool MainWindow::restore()
{
  const QByteArray saveData = QSettings().value(settingsKey()).toByteArray();
  if (!saveData.isEmpty())
    return restoreGeometry(saveData);

  setGeometry(screen()->availableGeometry());
  setWindowState(Qt::WindowState::WindowActive | Qt::WindowState::WindowMaximized);
  return false;
}

void MainWindow::save() const
{
  const QByteArray saveData = saveGeometry();
  QSettings().setValue(settingsKey(), saveData);
}

void MainWindow::setupRecentFiles(const QStringList &recentFiles) const
{
  if (recentFiles.isEmpty())
  {
    ui->menu_open_recent_world->setVisible(false);
    return;
  }
  ui->menu_open_recent_world->setVisible(true);
  auto i = recentFileActions.begin();
  auto end = recentFileActions.end();
  for (const QString &filePath : recentFiles)
  {
    QAction &action = **i;
    action.setVisible(true);
    action.setEnabled(true);
    action.setText(filePath);
    ++i;
  }

  for (; i != end; ++i)
  {
    QAction &action = **i;
    action.setEnabled(false);
    action.setVisible(false);
  }
}

void MainWindow::setWorldMenusEnabled(bool enabled) const
{
  ui->action_close_world->setEnabled(enabled);
  ui->action_save_world_details->setEnabled(enabled);
  ui->action_save_world_details_as->setEnabled(enabled);
  ui->action_edit_world_details->setEnabled(enabled);
  ui->action_edit_script_file->setEnabled(enabled);
  ui->action_reload_script_file->setEnabled(enabled);
  ui->action_log_session->setEnabled(enabled);
  ui->action_print->setEnabled(enabled);
  ui->action_undo->setEnabled(enabled);
  ui->action_redo->setEnabled(enabled);
  ui->action_paste->setEnabled(enabled);
  ui->action_select_all->setEnabled(enabled);
  ui->action_find->setEnabled(enabled);
  ui->action_find_again->setEnabled(enabled);
  ui->action_pause_output->setEnabled(enabled);
  ui->action_go_to_line->setEnabled(enabled);
  ui->action_command_history->setEnabled(enabled);
  ui->action_clear_output->setEnabled(enabled);
  ui->action_reset_all_timers->setEnabled(enabled);
  ui->action_stop_sound_playing->setEnabled(enabled);
  ui->action_server_status->setEnabled(enabled);
}

WorldTab *MainWindow::worldtab() const
{
  return qobject_cast<WorldTab *>(ui->world_tabs->currentWidget());
}

WorldTab *MainWindow::worldtab(int index) const
{
  return qobject_cast<WorldTab *>(ui->world_tabs->widget(index));
}

// Private slots

void MainWindow::onCopyAvailable(AvailableCopy copy)
{
  ui->action_cut->setEnabled(copy == AvailableCopy::Input);
  ui->action_copy->setEnabled(copy != AvailableCopy::None);
  ui->action_copy_as_html->setEnabled(copy == AvailableCopy::Output);
  ui->action_save_selection->setEnabled(copy != AvailableCopy::None);
}

void MainWindow::onConnectionStatusChanged(bool connected)
{
  ui->action_connect->setEnabled(!connected);
  ui->action_disconnect->setEnabled(connected);
}

void MainWindow::onNewActivity(WorldTab *tab)
{
  const int index = ui->world_tabs->indexOf(tab);
  if (index == -1 || index == ui->world_tabs->currentIndex())
    return;
  ui->world_tabs->tabBar()->setTabText(index, tab->title() + QStringLiteral(" ⏺︎"));
}

void MainWindow::on_action_about_triggered()
{
  QMessageBox box(this);
  box.setIconPixmap(QPixmap(QStringLiteral(":/appicon/appicon.svg")));
  box.setWindowTitle(ui->action_about->text());
  box.setText(QCoreApplication::applicationName());
  box.setInformativeText(tr("Version: %1").arg(QCoreApplication::applicationVersion()));
  box.exec();
}

void MainWindow::on_action_clear_output_triggered()
{
  worldtab()->ui->output->clear();
}

void MainWindow::on_action_close_all_notepad_windows_triggered()
{
  notepads->closeAll();
}

void MainWindow::on_action_close_world_triggered()
{
  QWidget *tab = ui->world_tabs->currentWidget();
  if (tab)
    tab->close();
}

void MainWindow::on_action_command_history_triggered()
{
  MudInput *input = worldtab()->ui->input;
  ListBox listbox(tr("Command History"), QString(), this);
  listbox.addItems(input->log());
  if (listbox.exec() != QDialog::Accepted)
    return;
  QString text = listbox.text();
  if (text.isEmpty())
    return;
  input->setText(text);
}

void MainWindow::on_action_copy_triggered()
{
  worldtab()->copyableEditor()->copy();
}

void MainWindow::on_action_copy_as_html_triggered()
{
  QString html = worldtab()->copyableEditor()->textCursor().selection().toHtml();
  QGuiApplication::clipboard()->setText(sanitizeHtml(html));
}

void MainWindow::on_action_cut_triggered()
{
  worldtab()->copyableEditor()->cut();
}

void MainWindow::on_action_connect_triggered()
{
  worldtab()->connectToHost();
}

void MainWindow::on_action_disconnect_triggered()
{
  worldtab()->disconnectFromHost();
}

void MainWindow::on_action_edit_script_file_triggered()
{
  worldtab()->editWorldScript();
}

void MainWindow::on_action_edit_world_details_triggered()
{
  WorldTab *tab = worldtab();
  tab->openWorldSettings();
  if (tab->isWindowModified())
    setWindowModified(true);
  const QString &title = tab->title();
  ui->world_tabs->setTabText(ui->world_tabs->currentIndex(), title);
  setWindowTitle(title + QStringLiteral("[*] - ") + QCoreApplication::applicationName());
}

void MainWindow::on_action_find_triggered()
{
  if (findDialog->exec() == QDialog::Accepted)
    findDialog->find(worldtab()->ui->output);
}

void MainWindow::on_action_find_again_triggered()
{
  if (findDialog->isFilled() || findDialog->exec() == QDialog::Accepted)
    findDialog->find(worldtab()->ui->output);
}

void MainWindow::on_action_global_preferences_triggered()
{
  SettingsDialog(settings, this).exec();
}

void MainWindow::on_action_go_to_line_triggered()
{
  QTextEdit *output = worldtab()->ui->output;
  QTextDocument *doc = output->document();
  const int blockCount = doc->blockCount();
  const int choice = QInputDialog::getInt(this, tr("Go to line"), tr("Enter line number"), blockCount, 1, blockCount);
  QTextCursor cursor(doc->findBlockByNumber(choice - 1));
  if (cursor.columnNumber())
    cursor.movePosition(QTextCursor::MoveOperation::NextRow);
  cursor.movePosition(QTextCursor::MoveOperation::EndOfBlock, QTextCursor::MoveMode::KeepAnchor);
  output->setTextCursor(cursor);
}

void MainWindow::on_action_log_session_triggered(bool checked)
{
  for (int i = 0, end = ui->world_tabs->count(); i < end; ++i)
  {
    if (checked)
      worldtab(i)->openLog();
    else
      worldtab(i)->closeLog();
  }
}

void MainWindow::on_action_new_triggered()
{
  const int currentIndex = ui->world_tabs->currentIndex();
  WorldTab *tab = createWorldTab(this);
  tab->createWorld();
  const int tabIndex = ui->world_tabs->addTab(tab, tr("New world"));
  ui->world_tabs->setCurrentIndex(tabIndex);
  if (tab->openWorldSettings())
  {
    setWindowModified(true);
    connectTab(tab);
    tab->start();
    return;
  }
  delete tab;
  ui->world_tabs->setCurrentIndex(currentIndex);
}

void MainWindow::on_action_new_window_triggered()
{
  MainWindow *newWindow = new MainWindow(notepads);
  newWindow->show();
}

void MainWindow::on_action_open_world_triggered()
{
  const QString filePath = QFileDialog::getOpenFileName(
      this,
      ui->action_open_world->text(),
      QStringLiteral(WORLDS_DIR),
      FileFilter::world());

  if (filePath.isEmpty())
    return;

  openWorld(filePath);
}

void MainWindow::on_action_paste_triggered()
{
  worldtab()->ui->input->paste();
}

void MainWindow::on_action_pause_output_triggered(bool checked)
{
  worldtab()->ui->output->verticalScrollBar()->setPaused(checked);
}

void MainWindow::on_action_print_triggered()
{
  QPrinter printer;
  QPrintDialog dialog(&printer, this);
  if (dialog.exec() != QDialog::Accepted)
    return;
  worldtab()->ui->output->document()->print(&printer);
}

void MainWindow::on_action_quit_triggered()
{
  QCoreApplication::quit();
}

void MainWindow::on_action_reload_script_file_triggered()
{
  worldtab()->reloadWorldScript();
}

void MainWindow::on_action_reset_all_timers_triggered()
{
  for (int i = 0, end = ui->world_tabs->count(); i < end; ++i)
    worldtab(i)->resetAllTimers();
}

void MainWindow::on_action_save_selection_triggered()
{
  const QString path = QFileDialog::getSaveFileName(
      this,
      tr("Save as"),
      QString(),
      FileFilter::text());

  if (path.isEmpty())
    return;

  QSaveFile file(path);
  if (file.open(QSaveFile::WriteOnly))
  {
    file.write(worldtab()->copyableEditor()->textCursor().selection().toPlainText().toUtf8());
    if (file.commit())
      return;
  }
  QErrorMessage::qtHandler()->showMessage(file.errorString());
  return;
}

void MainWindow::on_action_save_world_details_as_triggered()
{
  WorldTab *tab = worldtab();
  addRecentFile(tab->saveWorldAsNew());
  setWindowModified(tab->isWindowModified());
}

void MainWindow::on_action_save_world_details_triggered()
{
  WorldTab *tab = worldtab();
  addRecentFile(tab->saveWorld());
  setWindowModified(tab->isWindowModified());
}

void MainWindow::on_action_select_all_triggered()
{
  worldtab()->ui->input->selectAll();
}

void MainWindow::on_action_server_status_triggered()
{
  ServerStatus(worldtab()->serverStatus(), this).exec();
}

void MainWindow::on_action_status_bar_triggered(bool checked)
{
  ui->statusBar->setVisible(checked);
}

void MainWindow::on_action_stop_sound_playing_triggered()
{
  for (int i = 0, end = ui->world_tabs->count(); i < end; ++i)
    worldtab(i)->stopSound();
}

void MainWindow::on_action_wrap_output_triggered(bool checked)
{
  const QTextEdit::LineWrapMode mode =
      checked ? QTextEdit::LineWrapMode::WidgetWidth : QTextEdit::LineWrapMode::NoWrap;
  for (int i = 0, end = ui->world_tabs->count(); i < end; ++i)
    worldtab(i)->ui->output->setLineWrapMode(mode);
}

void MainWindow::on_action_visit_api_guide_triggered()
{
  QDesktopServices::openUrl(QStringLiteral("https://www.gammon.com.au/scripts/doc.php?general=function_list"));
}

void MainWindow::on_action_visit_bug_reports_triggered()
{
  QDesktopServices::openUrl(QStringLiteral("https://github.com/jnbooth/smushclient/issues"));
}

void MainWindow::on_menu_file_aboutToShow()
{
  WorldTab *tab = worldtab();
  const bool hasWorldScript = tab && !tab->world.getWorldScript().isEmpty();
  ui->action_edit_script_file->setEnabled(hasWorldScript);
  ui->action_reload_script_file->setEnabled(hasWorldScript);
}

void MainWindow::on_menu_help_aboutToShow()
{
  WorldTab *tab = worldtab();
  ui->action_server_status->setEnabled(!tab->serverStatus().isEmpty());
}

void MainWindow::on_menu_view_aboutToShow()
{
  WorldTab *tab = worldtab();
  ui->action_pause_output->setChecked(tab && tab->ui->output->verticalScrollBar()->paused());
}

void MainWindow::on_world_tabs_currentChanged(int index)
{
  WorldTab *lastTab = worldtab(lastTabIndex);
  if (lastTab)
  {
    lastTab->setIsActive(false);
    lastTab->setStatusBarVisible(false);
  }
  lastTabIndex = index;
  WorldTab *activeTab = worldtab(index);
  disconnect(socketConnection);
  if (!activeTab)
  {
    setWorldMenusEnabled(false);
    ui->action_connect->setEnabled(false);
    ui->action_disconnect->setEnabled(false);
    onCopyAvailable(AvailableCopy::None);
    return;
  }
  socketConnection = connect(activeTab, &WorldTab::connectionStatusChanged, this, &MainWindow::onConnectionStatusChanged);
  onConnectionStatusChanged(activeTab->connected());
  setWorldMenusEnabled(true);
  ui->action_pause_output->setChecked(activeTab->ui->output->verticalScrollBar()->paused());
  activeTab->setIsActive(true);
  activeTab->setStatusBarVisible(true);
  const QString &title = activeTab->title();
  ui->world_tabs->tabBar()->setTabText(index, title);
  setWindowTitle(title + QStringLiteral("[*] - ") + QCoreApplication::applicationName());
  setWindowModified(activeTab->isWindowModified());
  onCopyAvailable(activeTab->availableCopy());
}

void MainWindow::on_world_tabs_tabCloseRequested(int index)
{
  ui->world_tabs->widget(index)->close();
}
