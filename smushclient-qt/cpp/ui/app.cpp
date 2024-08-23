#include "app.h"
#include "ui_app.h"
#include "worldtab.h"

#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMainWindow>

App::App(QWidget *parent) : QMainWindow(parent), ui(new Ui::App)
{
  ui->setupUi(this);
  saveFilter = tr("World files (*.smush);;All Files (*.*)");
}

App::~App()
{
  delete ui;
}

void App::on_action_new_triggered()
{
  WorldTab *tab = new WorldTab(this);
  tab->createWorld();
  int tabIndex = ui->world_tabs->addTab(tab, tr("New world"));
  ui->world_tabs->setCurrentIndex(tabIndex);
  tab->openPreferences();
}

void App::on_action_open_world_triggered()
{
  QString dialogName = ui->action_open_world->text();
  QString filename = QFileDialog::getOpenFileName(this, dialogName, "", saveFilter);
  if (filename.isEmpty())
    return;

  WorldTab *tab = new WorldTab(ui->world_tabs);
  if (!tab->openWorld(filename))
  {
    delete tab;
    return;
  }
  int tabIndex = ui->world_tabs->addTab(tab, tab->title());
  ui->world_tabs->setCurrentIndex(tabIndex);
}

void App::on_action_world_properties_triggered()
{
  WorldTab *tab = (WorldTab *)ui->world_tabs->currentWidget();
  tab->openPreferences();
}

void App::on_action_save_world_details_triggered()
{
  WorldTab *tab = (WorldTab *)ui->world_tabs->currentWidget();
  if (tab == NULL)
  {
    return;
  }
  tab->saveWorld(saveFilter);
}

void App::on_action_save_world_details_as_triggered()
{
  WorldTab *tab = (WorldTab *)ui->world_tabs->currentWidget();
  if (tab == NULL)
  {
    return;
  }
  tab->saveWorldAsNew(saveFilter);
}

void App::on_world_tabs_currentChanged(int index)
{
  setWorldMenusEnabled(index != -1);
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
