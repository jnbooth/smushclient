#include "trigger.h"
#include "../ui/worlddetails/triggeredit.h"
#include "cxx-qt-gen/ffi.cxxqt.h"

TriggerModel::TriggerModel(SmushClient &client, QObject *parent)
    : AbstractSenderModel(client, SenderType::Trigger, parent)
{
  setHeaders(tr("Group/Label"), tr("Sequence"), tr("Pattern"), tr("Text"));
}

int TriggerModel::addItem(SmushClient &client, QWidget *parent)
{
  Trigger trigger;
  TriggerEdit edit(trigger, parent);
  if (edit.exec() == QDialog::Rejected)
    return -1;
  return client.addWorldTrigger(trigger);
}

int TriggerModel::editItem(SmushClient &client, size_t index, QWidget *parent)
{
  Trigger trigger(&client, index);
  TriggerEdit edit(trigger, parent);
  if (edit.exec() == QDialog::Rejected)
    return -2;
  const int result = client.replaceWorldTrigger(index, trigger);
  if (result < 0)
    return -3;
  if (edit.groupChanged())
    return -1;
  return result;
}

QString TriggerModel::exportXml(const SmushClient &client) const
{
  return client.exportWorldTriggers();
}

void TriggerModel::importXml(SmushClient &client, const QString &xml)
{
  client.importWorldTriggers(xml);
}
