#include "timers.h"
#include "timeredit.h"
#include "ui_timers.h"
#include "../../bridge/viewbuilder.h"
#include "../../fieldconnector.h"
#include "../../settings.h"

PrefsTimers::PrefsTimers(const World &world, SmushClient &client, Timekeeper *timekeeper, QWidget *parent)
    : AbstractPrefsTree(ModelType::Timer, parent),
      ui(new Ui::PrefsTimers),
      client(client),
      timekeeper(timekeeper)
{
  ui->setupUi(this);
  setTree(ui->tree);
  setHeaders({tr("Group/Label"), tr("Type"), tr("Occurrence"), tr("Text")});
  CONNECT_WORLD(EnableTimers);
  client.stopTimers();
  ui->tree->setModel(model());
  AbstractPrefsTree::buildTree();
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

void PrefsTimers::buildTree(ModelBuilder &builder)
{
  ui->item_count->setNum((int)client.buildTimersTree(builder, true));
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
