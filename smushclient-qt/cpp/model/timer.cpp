#include "timer.h"
#include "../ui/worlddetails/timeredit.h"
#include "smushclient_qt/src/bridge.cxxqt.h"

// Public methods

TimerModel::TimerModel(SmushClient &client, Timekeeper *timekeeper, QObject *parent)
    : AbstractSenderModel(client, SenderType::Timer, parent),
      timekeeper(timekeeper)
{
  setHeaders(tr("Group/Label"), tr("Type"), tr("Occurrence"), tr("Text"));
  client.stopTimers();
}

Qt::ItemFlags TimerModel::flags(const QModelIndex &index) const
{
  if (!index.constInternalPointer())
    return Qt::ItemFlag::ItemIsEnabled;

  const Qt::ItemFlags flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemNeverHasChildren;

  const int column = index.column();
  return column == 0 || column == 3 ? flags | Qt::ItemFlag::ItemIsEditable : flags;
}

// Protected overrides

int TimerModel::addItem(SmushClient &client, QWidget *parent)
{
  Timer timer;
  TimerEdit edit(timer, parent);
  if (edit.exec() == QDialog::Rejected)
    return EditResult::Unchanged;
  return client.addWorldTimer(timer, *timekeeper);
}

int TimerModel::editItem(SmushClient &client, size_t index, QWidget *parent)
{
  Timer timer(&client, index);
  TimerEdit edit(timer, parent);
  if (edit.exec() == QDialog::Rejected)
    return EditResult::Unchanged;
  const int result = client.replaceWorldTimer(index, timer, *timekeeper);
  if (result == (int)SenderAccessResult::Unchanged)
    return EditResult::Unchanged;
  if (result < 0)
    return EditResult::Failed;
  if (edit.groupChanged())
    return EditResult::GroupChanged;
  return result;
}

QString TimerModel::exportXml(const SmushClient &client) const
{
  return client.exportWorldTimers();
}

void TimerModel::importXml(SmushClient &client, const QString &xml)
{
  client.importWorldTimers(xml, *timekeeper);
}
