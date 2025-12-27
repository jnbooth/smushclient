#include "timers.h"
#include "../../fieldconnector.h"
#include "../../model/timer.h"
#include "smushclient_qt/src/ffi/world.cxxqt.h"
#include "timeredit.h"
#include "ui_timers.h"

PrefsTimers::PrefsTimers(const World& world, TimerModel* model, QWidget* parent)
  : AbstractPrefsTree(model, parent)
  , ui(new Ui::PrefsTimers)
{
  ui->setupUi(this);
  setTree(ui->tree);
  CONNECT_WORLD(EnableTimers);
}

PrefsTimers::~PrefsTimers()
{
  delete ui;
}

// Protected overrides

void
PrefsTimers::enableSingleButtons(bool enabled)
{
  ui->edit->setEnabled(enabled);
}

void
PrefsTimers::enableMultiButtons(bool enabled)
{
  ui->export_xml->setEnabled(enabled);
  ui->remove->setEnabled(enabled);
}
