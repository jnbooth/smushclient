#include "triggers.h"
#include "triggeredit.h"
#include "ui_triggers.h"
#include "../../bridge/viewbuilder.h"
#include "../../fieldconnector.h"

PrefsTriggers::PrefsTriggers(World &world, QWidget *parent)
    : AbstractPrefsTree(parent),
      ui(new Ui::PrefsTriggers),
      world(world)
{
  ui->setupUi(this);
  CONNECT_WORLD(EnableTriggers);
  ui->tree->clear();
  TreeBuilder builder(ui->tree);
  world.buildTriggerTree(builder);
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

  world.addTrigger(trigger);
  return true;
}

void PrefsTriggers::buildTree(TreeBuilder &builder)
{
  world.buildTriggerTree(builder);
}

bool PrefsTriggers::editItem(size_t index)
{
  Trigger trigger(&world, index);
  TriggerEdit edit(trigger, this);
  if (edit.exec() == QDialog::Rejected)
    return false;

  world.replaceTrigger(index, trigger);
  return true;
}

QString PrefsTriggers::exportXml() const
{
  return world.exportTriggers();
}

QString PrefsTriggers::importXml(const QString &xml)
{
  return world.importTriggers(xml);
}

void PrefsTriggers::removeItem(size_t index)
{
  world.removeTrigger(index);
}

void PrefsTriggers::setItemButtonsEnabled(bool enabled)
{
  ui->edit->setEnabled(enabled);
}

QTreeWidget *PrefsTriggers::tree() const
{
  return ui->tree;
}
