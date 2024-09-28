#include "app.h"
#include "ui_app.h"
#include "worldtab.h"
#include "../settings.h"

#include <QtWidgets/QFileDialog>

App::App(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::App),
      lastTabIndex(-1)
{
  ui->setupUi(this);
  saveFilter = tr("World files (*.smush);;All Files (*.*)");
  recentFileActions = QList<QAction *>{
      ui->action_rec_1,
      ui->action_rec_2,
      ui->action_rec_3,
      ui->action_rec_4,
      ui->action_rec_5};
  setupRecentFiles(Settings().recentFiles());
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
  ui->action_import->setEnabled(enabled);
  ui->action_plugins->setEnabled(enabled);
  ui->action_plugin_wizard->setEnabled(enabled);
  ui->action_save_world_details->setEnabled(enabled);
  ui->action_save_world_details_as->setEnabled(enabled);
  ui->action_print->setEnabled(enabled);
  ui->action_log_session->setEnabled(enabled);
  ui->action_reload_defaults->setEnabled(enabled);
  ui->action_world_properties->setEnabled(enabled);
  ui->action_undo->setEnabled(enabled);
  ui->action_paste->setEnabled(enabled);
  ui->action_paste_to_world->setEnabled(enabled);
  ui->action_recall_last_word->setEnabled(enabled);
  ui->action_select_all->setEnabled(enabled);
  ui->action_debug_packets->setEnabled(enabled);
  ui->action_go_to_matching_brace->setEnabled(enabled);
  ui->action_select_to_matching_brace->setEnabled(enabled);
}

// Private methods

WorldTab *App::worldtab() const
{
  return qobject_cast<WorldTab *>(ui->world_tabs->currentWidget());
}

WorldTab *App::worldtab(int index) const
{
  return qobject_cast<WorldTab *>(ui->world_tabs->widget(index));
}

// Slots

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
  const QString dialogName = ui->action_open_world->text();
  const QString filePath = QFileDialog::getOpenFileName(this, dialogName, QString(), saveFilter);
  if (filePath.isEmpty())
    return;

  openWorld(filePath);
}

void App::on_action_save_world_details_as_triggered()
{
  WorldTab *tab = worldtab();
  if (!tab)
    return;

  addRecentFile(tab->saveWorldAsNew(saveFilter));
}

void App::on_action_save_world_details_triggered()
{
  WorldTab *tab = worldtab();
  if (!tab)
    return;

  addRecentFile(tab->saveWorld(saveFilter));
}

void App::on_action_world_properties_triggered()
{
  WorldTab *tab = worldtab();
  if (!tab)
    return;
  tab->openWorldSettings();
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
