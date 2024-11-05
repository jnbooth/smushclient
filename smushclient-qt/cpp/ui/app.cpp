#include "app.h"
#include "ui_app.h"
#include <QtGui/QClipboard>
#include <QtGui/QTextDocumentFragment>
#include <QtGui/QDesktopServices>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QInputDialog>
#include "ui_worldtab.h"
#include "worldtab.h"
#include "../environment.h"
#include "../settings.h"
#include "../spans.h"
#include "../components/mudscrollbar.h"
#include "../scripting/listbox.h"
#include "finddialog.h"
#include "settings.h"

constexpr const char *saveFilter = "World files (*.smush);;All Files (*.*)";

App::App(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::App),
      lastTabIndex(-1),
      recentFileActions()
{
  findDialog = new FindDialog(this);
  if (QDir::current().isRoot())
  {
    QString dirPath = QCoreApplication::applicationDirPath();
    if (qsizetype i = dirPath.indexOf(QStringLiteral(".app")); i != -1)
      dirPath.truncate(dirPath.lastIndexOf(QDir::separator(), i));
    QDir::setCurrent(dirPath);
  }

  Settings settings;

  ui->setupUi(this);

  ui->action_status_bar->setChecked(settings.showStatusBar());
  ui->action_wrap_output->setChecked(settings.outputWrapping());
  ui->action_auto_connect->setChecked(settings.autoConnect());
  ui->action_reconnect_on_disconnect->setChecked(settings.reconnectOnDisconnect());
  connect(ui->action_maximize, &QAction::triggered, this, &App::showMaximized);
  connect(ui->action_minimize, &QAction::triggered, this, &App::showMinimized);
  connect(ui->action_restore, &QAction::triggered, this, &App::showNormal);

  recentFileActions = QList<QAction *>{
      ui->action_rec_1,
      ui->action_rec_2,
      ui->action_rec_3,
      ui->action_rec_4,
      ui->action_rec_5};

  ui->action_log_session->setChecked(settings.loggingEnabled());

  const QStringList recentFiles = settings.recentFiles();
  if (!recentFiles.empty())
    setupRecentFiles(recentFiles);
}

App::~App()
{
  QStringList lastFiles;
  const int tabCount = ui->world_tabs->count();
  lastFiles.reserve(tabCount);
  for (int i = 0; i < tabCount; ++i)
  {
    const QString &worldFilePath = worldtab(i)->worldFilePath();
    if (!worldFilePath.isEmpty())
      lastFiles.push_back(worldFilePath);
  }
  Settings().setLastFiles(lastFiles);
  delete ui;
}

void App::openWorld(const QString &filePath)
{
  WorldTab *tab = new WorldTab(ui->world_tabs);
  if (tab->openWorld(filePath))
  {
    connect(tab, &WorldTab::copyAvailable, this, &App::onCopyAvailable);
    const int tabIndex = ui->world_tabs->addTab(tab, tab->title());
    ui->world_tabs->setCurrentIndex(tabIndex);
    addRecentFile(filePath);
    tab->start();
    return;
  }
  delete tab;
  const RecentFileResult result = Settings().removeRecentFile(filePath);
  if (result.changed)
    setupRecentFiles(result.recentFiles);
}

// Private methods

void App::addRecentFile(const QString &filePath) const
{
  if (filePath.isEmpty())
    return;

  const RecentFileResult result = Settings().addRecentFile(filePath);
  if (!result.changed)
    return;

  setupRecentFiles(result.recentFiles);
}

void App::openRecentFile(qsizetype index)
{
  const QStringList recentFiles = Settings().recentFiles();
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

void App::setupRecentFiles(const QStringList &recentFiles) const
{
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

void App::setWorldMenusEnabled(bool enabled) const
{
  ui->action_close_world->setEnabled(enabled);
  ui->action_save_world_details->setEnabled(enabled);
  ui->action_save_world_details_as->setEnabled(enabled);
  ui->action_edit_world_details->setEnabled(enabled);
  ui->action_plugins->setEnabled(enabled);
  ui->action_edit_script_file->setEnabled(enabled);
  ui->action_reload_script_file->setEnabled(enabled);
  ui->action_log_session->setEnabled(enabled);
  ui->action_print->setEnabled(enabled);
  ui->action_undo->setEnabled(enabled);
  ui->action_redo->setEnabled(enabled);
  ui->action_paste->setEnabled(enabled);
  ui->action_select_all->setEnabled(enabled);
  ui->action_save_selection->setEnabled(enabled);
  ui->action_find->setEnabled(enabled);
  ui->action_find_again->setEnabled(enabled);
  ui->action_show_world_notepad->setEnabled(enabled);
  ui->action_pause_output->setEnabled(enabled);
  ui->action_connect->setEnabled(enabled);
  ui->action_disconnect->setEnabled(enabled);
  ui->action_go_to_line->setEnabled(enabled);
  ui->action_command_history->setEnabled(enabled);
  ui->action_clear_output->setEnabled(enabled);
  ui->action_reset_all_timers->setEnabled(enabled);
  ui->action_stop_sound_playing->setEnabled(enabled);
}

WorldTab *App::worldtab() const
{
  return qobject_cast<WorldTab *>(ui->world_tabs->currentWidget());
}

WorldTab *App::worldtab(int index) const
{
  return qobject_cast<WorldTab *>(ui->world_tabs->widget(index));
}

// Private slots

void App::onCopyAvailable(AvailableCopy copy)
{
  ui->action_cut->setEnabled(copy == AvailableCopy::Input);
  ui->action_copy->setEnabled(copy != AvailableCopy::None);
  ui->action_copy_as_html->setEnabled(copy == AvailableCopy::Output);
}

void App::on_action_auto_connect_triggered(bool checked)
{
  Settings().setAutoConnect(checked);
}

void App::on_action_clear_output_triggered()
{
  worldtab()->ui->output->clear();
}

void App::on_action_close_world_triggered()
{
  QWidget *tab = ui->world_tabs->currentWidget();
  if (tab)
    delete tab;
}

void App::on_action_command_history_triggered()
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

void App::on_action_copy_triggered()
{
  worldtab()->copyableEditor()->copy();
}

void App::on_action_copy_as_html_triggered()
{
  QString html = worldtab()->copyableEditor()->textCursor().selection().toHtml();
  QGuiApplication::clipboard()->setText(sanitizeHtml(html));
}

void App::on_action_cut_triggered()
{
  worldtab()->copyableEditor()->cut();
}

void App::on_action_connect_to_all_open_worlds_triggered()
{
  for (int i = 0, end = ui->world_tabs->count(); i < end; ++i)
    worldtab(i)->connectToHost();
}

void App::on_action_connect_triggered()
{
  worldtab()->connectToHost();
}

void App::on_action_disconnect_triggered()
{
  worldtab()->disconnectFromHost();
}

void App::on_action_edit_script_file_triggered()
{
  worldtab()->editWorldScript();
}

void App::on_action_edit_world_details_triggered()
{
  worldtab()->openWorldSettings();
}

void App::on_action_find_triggered()
{
  if (findDialog->exec() == QDialog::Accepted)
    findDialog->find(worldtab()->ui->output);
}

void App::on_action_find_again_triggered()
{
  if (findDialog->isFilled() || findDialog->exec() == QDialog::Accepted)
    findDialog->find(worldtab()->ui->output);
}

void App::on_action_global_preferences_triggered()
{
  Settings settings;
  SettingsDialog dialog(settings, this);
  for (int i = 0, end = ui->world_tabs->count(); i < end; ++i)
  {
    WorldTab *tab = worldtab(i);
    connect(&dialog, &SettingsDialog::inputBackgroundChanged, tab, &WorldTab::onInputBackgroundChanged);
    connect(&dialog, &SettingsDialog::inputFontChanged, tab, &WorldTab::onInputFontChanged);
    connect(&dialog, &SettingsDialog::inputForegroundChanged, tab, &WorldTab::onInputForegroundChanged);
    connect(&dialog, &SettingsDialog::outputFontChanged, tab, &WorldTab::onOutputFontChanged);
  }
  dialog.exec();
}

void App::on_action_go_to_line_triggered()
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

void App::on_action_log_session_triggered(bool checked)
{
  Settings().setLoggingEnabled(checked);
  for (int i = 0, end = ui->world_tabs->count(); i < end; ++i)
  {
    if (checked)
      worldtab(i)->openLog();
    else
      worldtab(i)->closeLog();
  }
}

void App::on_action_new_triggered()
{
  const int currentIndex = ui->world_tabs->currentIndex();
  WorldTab *tab = new WorldTab(this);
  tab->createWorld();
  connect(tab, &WorldTab::copyAvailable, this, &App::onCopyAvailable);
  const int tabIndex = ui->world_tabs->addTab(tab, tr("New world"));
  ui->world_tabs->setCurrentIndex(tabIndex);
  if (tab->openWorldSettings())
  {
    tab->start();
    return;
  }
  delete tab;
  ui->world_tabs->setCurrentIndex(currentIndex);
}

void App::on_action_new_window_triggered()
{
  App *newWindow = new App();
  newWindow->show();
}

void App::on_action_open_world_triggered()
{
  const QString filePath = QFileDialog::getOpenFileName(
      this,
      ui->action_open_world->text(),
      QStringLiteral(WORLDS_DIR),
      tr(saveFilter));

  if (filePath.isEmpty())
    return;

  openWorld(filePath);
}

void App::on_action_paste_triggered()
{
  worldtab()->ui->input->paste();
}

void App::on_action_pause_output_triggered(bool checked)
{
  worldtab()->ui->output->verticalScrollBar()->setPaused(checked);
}

void App::on_action_plugins_triggered()
{
  worldtab()->openPluginsDialog();
}

void App::on_action_quit_triggered()
{
  QCoreApplication::quit();
}

void App::on_action_reconnect_on_disconnect_triggered(bool checked)
{
  Settings().setReconnectOnDisconnect(checked);
}

void App::on_action_reload_script_file_triggered()
{
  worldtab()->reloadWorldScript();
}

void App::on_action_reset_all_timers_triggered()
{
  for (int i = 0, end = ui->world_tabs->count(); i < end; ++i)
    worldtab(i)->resetAllTimers();
}

void App::on_action_save_world_details_as_triggered()
{
  addRecentFile(worldtab()->saveWorldAsNew(tr(saveFilter)));
}

void App::on_action_select_all_triggered()
{
  worldtab()->ui->input->selectAll();
}

void App::on_action_status_bar_triggered(bool checked)
{
  Settings().setShowStatusBar(checked);
  ui->statusBar->setVisible(checked);
}

void App::on_action_stop_sound_playing_triggered()
{
  for (int i = 0, end = ui->world_tabs->count(); i < end; ++i)
    worldtab(i)->stopSound();
}

void App::on_action_wrap_output_triggered(bool checked)
{
  Settings().setOutputWrapping(checked);
  const QTextEdit::LineWrapMode mode =
      checked ? QTextEdit::LineWrapMode::WidgetWidth : QTextEdit::LineWrapMode::NoWrap;
  for (int i = 0, end = ui->world_tabs->count(); i < end; ++i)
    worldtab(i)->ui->output->setLineWrapMode(mode);
}

void App::on_action_save_world_details_triggered()
{
  addRecentFile(worldtab()->saveWorld(tr(saveFilter)));
}

void App::on_action_visit_api_guide_triggered()
{
  QDesktopServices::openUrl(QStringLiteral("https://www.gammon.com.au/scripts/doc.php?general=function_list"));
}

void App::on_action_visit_bug_reports_triggered()
{
  QDesktopServices::openUrl(QStringLiteral("https://github.com/jnbooth/smushclient/issues"));
}

void App::on_menu_file_aboutToShow()
{
  WorldTab *tab = worldtab();
  const bool hasWorldScript = tab && !tab->world.getWorldScript().isEmpty();
  ui->action_edit_script_file->setEnabled(hasWorldScript);
  ui->action_reload_script_file->setEnabled(hasWorldScript);
}

void App::on_menu_view_aboutToShow()
{
  WorldTab *tab = worldtab();
  ui->action_pause_output->setChecked(tab && tab->ui->output->verticalScrollBar()->paused());
}

void App::on_world_tabs_currentChanged(int index)
{
  const WorldTab *lastTab = worldtab(lastTabIndex);
  if (lastTab)
    lastTab->onTabSwitch(false);
  lastTabIndex = index;
  const WorldTab *activeTab = worldtab(index);
  if (!activeTab)
  {
    setWorldMenusEnabled(false);
    onCopyAvailable(AvailableCopy::None);
    return;
  }
  setWorldMenusEnabled(true);
  ui->action_pause_output->setChecked(activeTab->ui->output->verticalScrollBar()->paused());
  activeTab->onTabSwitch(true);
  onCopyAvailable(activeTab->availableCopy());
}

void App::on_world_tabs_tabCloseRequested(int index)
{
  delete ui->world_tabs->widget(index);
}
