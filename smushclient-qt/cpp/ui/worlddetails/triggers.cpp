#include "triggers.h"
#include "triggeredit.h"
#include "ui_triggers.h"
#include "../../bridge/viewbuilder.h"
#include "../../fieldconnector.h"

PrefsTriggers::PrefsTriggers(const World &world, SmushClient &client, QWidget *parent)
    : AbstractPrefsTree(parent),
      ui(new Ui::PrefsTriggers),
      client(client)
{
  ui->setupUi(this);
  CONNECT_WORLD(EnableTriggers);
  client.stopTriggers();
  ui->tree->clear();
  TreeBuilder builder(ui->tree);
  ui->item_count->setNum((int)client.buildTriggersTree(builder));
}

PrefsTriggers::~PrefsTriggers()
{
  delete ui;
}

// Protected overrides

bool PrefsTriggers::addItem()
{
  Trigger trigger;
  TriggerEdit edit(trigger, this);
  if (edit.exec() == QDialog::Rejected)
    return false;

  client.addWorldTrigger(trigger);
  return true;
}

void PrefsTriggers::buildTree(TreeBuilder &builder)
{
  ui->item_count->setNum((int)client.buildTriggersTree(builder));
}

bool PrefsTriggers::editItem(size_t index)
{
  Trigger trigger(&client, index);
  TriggerEdit edit(trigger, this);
  if (edit.exec() == QDialog::Rejected)
    return false;

  client.replaceWorldTrigger(index, trigger);
  return true;
}

QString PrefsTriggers::exportXml() const
{
  return client.exportWorldTriggers();
}

void PrefsTriggers::importXml(const QString &xml)
{
  client.importWorldTriggers(xml);
}

void PrefsTriggers::removeItem(size_t index)
{
  client.removeWorldTrigger(index);
}

void PrefsTriggers::setItemButtonsEnabled(bool enabled)
{
  ui->edit->setEnabled(enabled);
}

QTreeWidget *PrefsTriggers::tree() const
{
  return ui->tree;
}
