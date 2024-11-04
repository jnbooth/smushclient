#include "app.h"
#include "ui_app.h"
#include "ui_worldtab.h"
#include "worldtab.h"
#include "../environment.h"
#include "../settings.h"
#include "finddialog.h"
#include "settings.h"

#include <QtWidgets/QFileDialog>

App::App(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::App),
      lastTabIndex(-1),
      recentFileActions(),
      saveFilter(tr("World files (*.smush);;All Files (*.*)"))
{
  findDialog = new FindDialog(this);
  if (QDir::current().isRoot())
  {
    QString dirPath = QCoreApplication::applicationDirPath();
    if (qsizetype i = dirPath.indexOf(QStringLiteral(".app")); i != -1)
      dirPath.truncate(dirPath.lastIndexOf(QDir::separator(), i));
    QDir::setCurrent(dirPath);
  }
  ui->setupUi(this);
  recentFileActions = QList<QAction *>{
      ui->action_rec_1,
      ui->action_rec_2,
      ui->action_rec_3,
      ui->action_rec_4,
      ui->action_rec_5};
  const QStringList recentFiles = Settings().recentFiles();
  if (recentFiles.empty())
    return;

  setupRecentFiles(recentFiles);
  openWorld(recentFiles.constFirst());
}

App::~App()
{
  delete ui;
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

void App::openRecentFile(qsizetype index) const
{
  const QStringList recentFiles = Settings().recentFiles();
  if (index >= recentFiles.length())
    return;
  openWorld(recentFiles.at(index));
}

void App::openWorld(const QString &filePath) const
{
  WorldTab *tab = new WorldTab(ui->world_tabs);
  if (tab->openWorld(filePath))
  {
    const int tabIndex = ui->world_tabs->addTab(tab, tab->title());
    ui->world_tabs->setCurrentIndex(tabIndex);
    addRecentFile(filePath);
    return;
  }
  delete tab;
  const RecentFileResult result = Settings().removeRecentFile(filePath);
  if (result.changed)
    setupRecentFiles(result.recentFiles);
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
  ui->action_cut->setEnabled(enabled);
  ui->action_copy->setEnabled(enabled);
  ui->action_copy_as_html->setEnabled(enabled);
  ui->action_paste->setEnabled(enabled);
  ui->action_paste_to_world->setEnabled(enabled);
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

void App::on_action_close_world_triggered()
{
  QWidget *tab = ui->world_tabs->currentWidget();
  if (tab)
    delete tab;
}

void App::on_action_connect_triggered()
{
  if (WorldTab *tab = worldtab(); tab)
    tab->connectToHost();
}

void App::on_action_disconnect_triggered()
{
  if (WorldTab *tab = worldtab(); tab)
    tab->disconnectFromHost();
}

void App::on_action_edit_script_file_triggered()
{
  if (WorldTab *tab = worldtab(); tab)
    tab->editWorldScript();
}

void App::on_action_edit_world_details_triggered()
{
  if (WorldTab *tab = worldtab(); tab)
    tab->openWorldSettings();
}

void App::on_action_find_triggered()
{
  if (WorldTab *tab = worldtab(); tab && findDialog->exec() == QDialog::Accepted)
    findDialog->find(tab->ui->output);
}

void App::on_action_find_again_triggered()
{
  if (WorldTab *tab = worldtab(); tab && (findDialog->isFilled() || findDialog->exec() == QDialog::Accepted))
    findDialog->find(tab->ui->output);
}

void App::on_action_global_preferences_triggered()
{
  Settings settings;
  SettingsDialog dialog(settings, this);
  for (QObject *child : ui->world_tabs->children())
  {
    WorldTab *tab = qobject_cast<WorldTab *>(child);
    if (!tab)
      continue;
    connect(&dialog, &SettingsDialog::inputBackgroundChanged, tab, &WorldTab::onInputBackgroundChanged);
    connect(&dialog, &SettingsDialog::inputFontChanged, tab, &WorldTab::onInputFontChanged);
    connect(&dialog, &SettingsDialog::inputForegroundChanged, tab, &WorldTab::onInputForegroundChanged);
    connect(&dialog, &SettingsDialog::outputFontChanged, tab, &WorldTab::onOutputFontChanged);
  }
  dialog.exec();
}

void App::on_action_new_triggered()
{
  WorldTab *tab = new WorldTab(this);
  tab->createWorld();
  const int tabIndex = ui->world_tabs->addTab(tab, tr("New world"));
  ui->world_tabs->setCurrentIndex(tabIndex);
  tab->openWorldSettings();
}

void App::on_action_open_world_triggered()
{
  const QString filePath = QFileDialog::getOpenFileName(
      this,
      ui->action_open_world->text(),
      QStringLiteral(WORLDS_DIR),
      saveFilter);
  if (filePath.isEmpty())
    return;

  openWorld(filePath);
}

void App::on_action_plugins_triggered()
{
  if (WorldTab *tab = worldtab(); tab)
    tab->openPluginsDialog();
}

void App::on_action_quit_triggered()
{
  QCoreApplication::quit();
}

void App::on_action_reload_script_file_triggered()
{
  if (WorldTab *tab = worldtab(); tab)
    tab->reloadWorldScript();
}

void App::on_action_save_world_details_as_triggered()
{
  if (WorldTab *tab = worldtab(); tab)
    addRecentFile(tab->saveWorldAsNew(saveFilter));
}

void App::on_action_save_world_details_triggered()
{
  if (WorldTab *tab = worldtab(); tab)
    addRecentFile(tab->saveWorld(saveFilter));
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
    return;
  }
  setWorldMenusEnabled(true);
  activeTab->onTabSwitch(true);
}

void App::on_world_tabs_tabCloseRequested(int index)
{
  delete ui->world_tabs->widget(index);
}
