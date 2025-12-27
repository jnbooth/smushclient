#include "triggers.h"
#include "../../fieldconnector.h"
#include "../../model/trigger.h"
#include "smushclient_qt/src/ffi/world.cxxqt.h"
#include "triggeredit.h"
#include "ui_triggers.h"

PrefsTriggers::PrefsTriggers(const World& world,
                             TriggerModel* model,
                             QWidget* parent)
  : AbstractPrefsTree(model, parent)
  , ui(new Ui::PrefsTriggers)
{
  ui->setupUi(this);
  setTree(ui->tree);
  CONNECT_WORLD(EnableTriggers);
}

PrefsTriggers::~PrefsTriggers()
{
  delete ui;
}

// Protected overrides

void
PrefsTriggers::enableSingleButtons(bool enabled)
{
  ui->edit->setEnabled(enabled);
}

void
PrefsTriggers::enableMultiButtons(bool enabled)
{
  ui->export_xml->setEnabled(enabled);
  ui->remove->setEnabled(enabled);
}
