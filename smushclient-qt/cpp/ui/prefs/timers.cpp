#include "timers.h"
#include "ui_timers.h"

PrefsTimers::PrefsTimers(const World *world, QWidget *parent)
    : AbstractPrefsPane(parent), ui(new Ui::PrefsTimers)
{
  ui->setupUi(this);
  CONNECT_WORLD(EnableTimers);
}

PrefsTimers::~PrefsTimers()
{
  delete ui;
}
