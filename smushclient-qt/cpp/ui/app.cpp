#include "app.h"
#include "../settings.h"
#include "ui_app.h"
#include "worldtab.h"

#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMainWindow>

App::App(QWidget *parent) : QMainWindow(parent), ui(new Ui::App)
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

void App::addRecentFile(const QString &filePath)
{
  if (filePath.isEmpty())
    return;

  RecentFileResult result = Settings().addRecentFile(filePath);
  if (!result.changed)
    return;

  setupRecentFiles(result.recentFiles);
}

void App::openRecentFile(qsizetype index)
{
  QStringList recentFiles = Settings().recentFiles();
  if (index >= recentFiles.length())
    return;
  openWorld(recentFiles.at(index));
}

void App::openWorld(const QString &filePath)
{
  WorldTab *tab = new WorldTab(ui->world_tabs);
  if (tab->openWorld(filePath))
  {
    int tabIndex = ui->world_tabs->addTab(tab, tab->title());
    ui->world_tabs->setCurrentIndex(tabIndex);
    return;
  }
  delete tab;
  RecentFileResult result = Settings().removeRecentFile(filePath);
  if (result.changed)
    setupRecentFiles(result.recentFiles);
}

void App::setupRecentFiles(const QStringList &recentFiles)
{
  auto i = recentFileActions.begin();
  auto end = recentFileActions.end();
  for (QString filePath : recentFiles)
  {
    QAction *action = *i;
    action->setVisible(true);
    action->setEnabled(true);
    action->setText(filePath);
    ++i;
  }

  for (; i != end; ++i)
  {
    QAction *action = *i;
    action->setEnabled(false);
    action->setVisible(false);
  }
}

void App::setWorldMenusEnabled(bool enabled)
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

// Slots

void App::on_action_new_triggered()
{
  WorldTab *tab = new WorldTab(this);
  tab->createWorld();
  int tabIndex = ui->world_tabs->addTab(tab, tr("New world"));
  ui->world_tabs->setCurrentIndex(tabIndex);
  tab->openWorldSettings();
}

void App::on_action_open_world_triggered()
{
  QString dialogName = ui->action_open_world->text();
  QString filePath = QFileDialog::getOpenFileName(this, dialogName, "", saveFilter);
  if (filePath.isEmpty())
    return;

  openWorld(filePath);
}

void App::on_action_save_world_details_as_triggered()
{
  WorldTab *tab = (WorldTab *)ui->world_tabs->currentWidget();
  if (tab == nullptr)
    return;

  addRecentFile(tab->saveWorldAsNew(saveFilter));
}

void App::on_action_save_world_details_triggered()
{
  WorldTab *tab = (WorldTab *)ui->world_tabs->currentWidget();
  if (tab == nullptr)
    return;

  addRecentFile(tab->saveWorld(saveFilter));
}

void App::on_action_world_properties_triggered()
{
  WorldTab *tab = (WorldTab *)ui->world_tabs->currentWidget();
  tab->openWorldSettings();
}

void App::on_world_tabs_currentChanged(int index)
{
  setWorldMenusEnabled(index != -1);
}
