#include "app.h"
#include "ui_app.h"
#include "worldprefs.h"

#include <QtWidgets/QMainWindow>

App::App(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::App)
{
    ui->setupUi(this);
}

App::~App()
{
    delete ui;
}

void App::on_action_new_triggered()
{
  WorldPrefs world;
  world.exec();
}

