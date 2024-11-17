#include "timer.h"
#include "../ui/worlddetails/timeredit.h"
#include "cxx-qt-gen/ffi.cxxqt.h"

TimerModel::TimerModel(SmushClient &client, Timekeeper *timekeeper, QObject *parent)
    : AbstractSenderModel(client, SenderType::Timer, parent),
      timekeeper(timekeeper)
{
  setHeaders(tr("Group/Label"), tr("Type"), tr("Occurrence"), tr("Text"));
}

int TimerModel::addItem(SmushClient &client, QWidget *parent)
{
  Timer timer;
  TimerEdit edit(timer, parent);
  if (edit.exec() == QDialog::Rejected)
    return -1;
  return client.addWorldTimer(timer, *timekeeper);
}

int TimerModel::editItem(SmushClient &client, size_t index, QWidget *parent)
{
  Timer timer(&client, index);
  TimerEdit edit(timer, parent);
  if (edit.exec() == QDialog::Rejected)
    return -1;
  const int result = client.replaceWorldTimer(index, timer, *timekeeper);
  if (result < 0)
    return -3;
  if (edit.groupChanged())
    return -2;
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
