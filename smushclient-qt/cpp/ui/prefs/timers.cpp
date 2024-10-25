#include "timers.h"
#include "timeredit.h"
#include "ui_timers.h"
#include "../../bridge/viewbuilder.h"
#include "../../fieldconnector.h"

PrefsTimers::PrefsTimers(World &world, QWidget *parent)
    : AbstractPrefsTree(parent),
      ui(new Ui::PrefsTimers),
      world(world)
{
  ui->setupUi(this);
  CONNECT_WORLD(EnableTimers);
  ui->tree->clear();
  TreeBuilder builder(ui->tree);
  world.buildTimerTree(builder);
}

PrefsTimers::~PrefsTimers()
{
  delete ui;
}

// Protected overrides

bool PrefsTimers::addItem()
{
  Timer timer;
  TimerEdit edit(timer, this);
  if (edit.exec() == QDialog::Rejected)
    return false;

  world.addTimer(timer);
  return true;
}

void PrefsTimers::buildTree(TreeBuilder &builder)
{
  world.buildTimerTree(builder);
}

bool PrefsTimers::editItem(size_t index)
{
  Timer timer(&world, index);
  TimerEdit edit(timer, this);
  if (edit.exec() == QDialog::Rejected)
    return false;

  world.replaceTimer(index, timer);
  return true;
}

QString PrefsTimers::exportXml() const
{
  return world.exportTimers();
}

QString PrefsTimers::importXml(const QString &xml)
{
  return world.importTimers(xml);
}

void PrefsTimers::removeItem(size_t index)
{
  world.removeTimer(index);
}

void PrefsTimers::setItemButtonsEnabled(bool enabled)
{
  ui->edit->setEnabled(enabled);
}

QTreeWidget *PrefsTimers::tree() const
{
  return ui->tree;
}
