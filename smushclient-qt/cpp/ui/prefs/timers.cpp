#include "../../fieldconnector.h"
#include "../../bridge/treebuilder.h"
#include "timers.h"
#include "ui_timers.h"
#include "timeredit.h"

PrefsTimers::PrefsTimers(World &world, QWidget *parent)
    : AbstractPrefsTree(parent),
      ui(new Ui::PrefsTimers),
      world(world)
{
  ui->setupUi(this);
  CONNECT_WORLD(EnableTimers);
  buildTree();
}

PrefsTimers::~PrefsTimers()
{
  delete ui;
}

// Protected overrides

void PrefsTimers::addItem()
{
  Timer timer;
  TimerEdit edit(timer, this);
  if (edit.exec() == QDialog::Rejected)
    return;

  world.addTimer(timer);
  buildTree();
}

void PrefsTimers::editItem(size_t index)
{
  Timer timer;
  world.getTimer(index, timer);
  TimerEdit edit(timer, this);
  if (edit.exec() == QDialog::Rejected)
    return;

  world.replaceTimer(index, timer);
  buildTree();
}

void PrefsTimers::removeItem(size_t index)
{
  world.removeTimer(index);
  buildTree();
}

void PrefsTimers::setItemButtonsEnabled(bool enabled)
{
  ui->edit->setEnabled(enabled);
}

QTreeWidget *PrefsTimers::tree() const
{
  return ui->tree;
}

// Private methods

void PrefsTimers::buildTree()
{
  ui->tree->clear();
  TreeBuilder builder(ui->tree);
  world.buildTriggerTree(builder);
}
