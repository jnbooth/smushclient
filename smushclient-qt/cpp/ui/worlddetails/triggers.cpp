#include "triggers.h"
#include "triggeredit.h"
#include "ui_triggers.h"
#include "../../model/trigger.h"
#include "../../fieldconnector.h"

PrefsTriggers::PrefsTriggers(const World &world, SmushClient &client, QWidget *parent)
    : AbstractPrefsTree(parent),
      ui(new Ui::PrefsTriggers)
{
  ui->setupUi(this);
  model = new TriggerModel(client, this);
  setModel(model);
  setTree(ui->tree);
  CONNECT_WORLD(EnableTriggers);
  client.stopTriggers();
}

PrefsTriggers::~PrefsTriggers()
{
  delete ui;
}

// Protected overrides

void PrefsTriggers::enableSingleButtons(bool enabled)
{
  ui->edit->setEnabled(enabled);
}

void PrefsTriggers::enableMultiButtons(bool enabled)
{
  ui->export_xml->setEnabled(enabled);
  ui->remove->setEnabled(enabled);
}
