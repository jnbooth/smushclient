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
  Trigger trigger(&world, index);
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
  ui->tree->clear();
  TreeBuilder builder(ui->tree);
  world.buildTriggerTree(builder);
}
