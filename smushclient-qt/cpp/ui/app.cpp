#include "app.h"
#include "ui_app.h"
#include "worldtab.h"

#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMainWindow>

App::App(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::App)
{
  ui->setupUi(this);
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
  QString filename = QFileDialog::getOpenFileName(this, tr("Open world"), "", tr("World files (*.qmc);;All Files (*.*)"));
  if (filename.isEmpty())
  {
    return;
  }
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
