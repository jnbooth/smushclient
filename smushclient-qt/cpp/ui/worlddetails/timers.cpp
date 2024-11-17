#include "timers.h"
#include "timeredit.h"
#include "ui_timers.h"
#include "../../fieldconnector.h"
#include "../../model/timer.h"

PrefsTimers::PrefsTimers(const World &world, SmushClient &client, Timekeeper *timekeeper, QWidget *parent)
    : AbstractPrefsTree(parent),
      ui(new Ui::PrefsTimers)
{
  ui->setupUi(this);
  model = new TimerModel(client, timekeeper, this);
  setModel(model);
  setTree(ui->tree);
  CONNECT_WORLD(EnableTimers);
  client.stopTimers();
}

PrefsTimers::~PrefsTimers()
{
  delete ui;
}

// Protected overrides

void PrefsTimers::enableSingleButtons(bool enabled)
{
  ui->edit->setEnabled(enabled);
}

void PrefsTimers::enableMultiButtons(bool enabled)
{
  ui->export_xml->setEnabled(enabled);
  ui->remove->setEnabled(enabled);
}
