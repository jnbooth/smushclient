#include "../../fieldconnector.h"
#include "../../viewbuilder.h"
#include "timers.h"
#include "ui_timers.h"
#include "timeredit.h"

PrefsTimers::PrefsTimers(World *world, QWidget *parent)
    : AbstractPrefsTree(parent), ui(new Ui::PrefsTimers), world(world), builder(nullptr)
{
  ui->setupUi(this);
  builder = TreeBuilder(ui->tree);
  CONNECT_WORLD(EnableTimers);

  buildTree();
}

PrefsTimers::~PrefsTimers()
{
  delete ui;
}

void PrefsTimers::buildTree()
{
  builder.clear();
  world->buildTriggerTree(builder);
}

void PrefsTimers::addItem()
{
  Timer timer;
  TimerEdit edit(&timer, this);
  if (edit.exec() == QDialog::Rejected)
    return;

  world->addTimer(timer);
  buildTree();
}

void PrefsTimers::editItem(size_t index)
{
  Timer timer;
  world->getTimer(index, timer);
  TimerEdit edit(&timer, this);
  if (edit.exec() == QDialog::Rejected)
    return;

  world->replaceTimer(index, timer);
  buildTree();
}

void PrefsTimers::removeItem(size_t index)
{
  world->removeTimer(index);
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
