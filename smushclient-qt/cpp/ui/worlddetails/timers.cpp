#include "timers.h"
#include "timeredit.h"
#include "ui_timers.h"
#include "../../bridge/viewbuilder.h"
#include "../../fieldconnector.h"

PrefsTimers::PrefsTimers(const World &world, SmushClient &client, Timekeeper *timekeeper, QWidget *parent)
    : AbstractPrefsTree(parent),
      ui(new Ui::PrefsTimers),
      client(client),
      timekeeper(timekeeper)
{
  ui->setupUi(this);
  CONNECT_WORLD(EnableTimers);
  client.stopTimers();
  ui->tree->clear();
  TreeBuilder builder(ui->tree);
  client.buildTimersTree(builder);
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

  client.addWorldTimer(timer, *timekeeper);
  return true;
}

void PrefsTimers::buildTree(TreeBuilder &builder)
{
  client.buildTimersTree(builder);
}

bool PrefsTimers::editItem(size_t index)
{
  Timer timer(&client, index);
  TimerEdit edit(timer, this);
  if (edit.exec() == QDialog::Rejected)
    return false;

  client.replaceWorldTimer(index, timer, *timekeeper);
  return true;
}

QString PrefsTimers::exportXml() const
{
  return client.exportWorldTimers();
}

void PrefsTimers::importXml(const QString &xml)
{
  client.importWorldTimers(xml, *timekeeper);
}

void PrefsTimers::removeItem(size_t index)
{
  client.removeWorldTimer(index);
}

void PrefsTimers::setItemButtonsEnabled(bool enabled)
{
  ui->edit->setEnabled(enabled);
}

QTreeWidget *PrefsTimers::tree() const
{
  return ui->tree;
}
