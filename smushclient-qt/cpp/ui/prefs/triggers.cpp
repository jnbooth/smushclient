#include "triggers.h"
#include "ui_triggers.h"

PrefsTriggers::PrefsTriggers(const World *world, QWidget *parent)
    : AbstractPrefsPane(parent), ui(new Ui::PrefsTriggers)
{
  ui->setupUi(this);
  CONNECT_WORLD(EnableTriggers);
  CONNECT_WORLD(EnableTriggerSounds);
}

PrefsTriggers::~PrefsTriggers()
{
  delete ui;
}
