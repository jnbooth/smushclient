#include "../../fieldconnector.h"
#include "../../viewbuilder.h"
#include "triggers.h"
#include "ui_triggers.h"
#include "triggeredit.h"

PrefsTriggers::PrefsTriggers(World &world, QWidget *parent)
    : AbstractPrefsTree(parent), ui(new Ui::PrefsTriggers), world(world), builder(nullptr)
{
  ui->setupUi(this);
  builder = TreeBuilder(ui->tree);
  CONNECT_WORLD(EnableTriggers);
  CONNECT_WORLD(EnableTriggerSounds);

  buildTree();
}

PrefsTriggers::~PrefsTriggers()
{
  delete ui;
}

// Protected overrides

void PrefsTriggers::addItem()
{
  Trigger trigger;
  TriggerEdit edit(trigger, this);
  if (edit.exec() == QDialog::Rejected)
    return;

  world.addTrigger(trigger);
  buildTree();
}

void PrefsTriggers::editItem(size_t index)
{
  Trigger trigger;
  world.getTrigger(index, trigger);
  TriggerEdit edit(trigger, this);
  if (edit.exec() == QDialog::Rejected)
    return;

  world.replaceTrigger(index, trigger);
  buildTree();
}

void PrefsTriggers::removeItem(size_t index)
{
  world.removeTrigger(index);
  buildTree();
}

void PrefsTriggers::setItemButtonsEnabled(bool enabled)
{
  ui->edit->setEnabled(enabled);
}

QTreeWidget *PrefsTriggers::tree() const
{
  return ui->tree;
}

// Private methods

void PrefsTriggers::buildTree()
{
  builder.clear();
  world.buildTriggerTree(builder);
}
