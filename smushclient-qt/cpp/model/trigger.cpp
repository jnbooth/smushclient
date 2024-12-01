#include "trigger.h"
#include "../ui/worlddetails/triggeredit.h"
#include "smushclient_qt/src/bridge.cxxqt.h"

// Public methods

TriggerModel::TriggerModel(SmushClient &client, QObject *parent)
    : AbstractSenderModel(client, SenderType::Trigger, parent)
{
  setHeaders(tr("Group/Label"), tr("Sequence"), tr("Pattern"), tr("Text"));
  client.stopTriggers();
}

// Public overrides

QString TriggerModel::exportXml() const
{
  return client.exportWorldTriggers();
}


Qt::ItemFlags TriggerModel::flags(const QModelIndex &index) const
{
  if (!index.constInternalPointer())
    return Qt::ItemFlag::ItemIsEnabled;

  return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemNeverHasChildren | Qt::ItemIsEditable;
}

// Protected overrides

int TriggerModel::add(QWidget *parent)
{
  Trigger trigger;
  TriggerEdit edit(trigger, parent);
  if (edit.exec() == QDialog::Rejected)
    return EditResult::Unchanged;
  return client.addWorldTrigger(trigger);
}

int TriggerModel::edit(size_t index, QWidget *parent)
{
  Trigger trigger(&client, index);
  TriggerEdit edit(trigger, parent);
  if (edit.exec() == QDialog::Rejected)
    return EditResult::Unchanged;
  const int result = client.replaceWorldTrigger(index, trigger);
  if (result == (int)SenderAccessResult::Unchanged)
    return EditResult::Unchanged;
  if (result < 0)
    return EditResult::Failed;
  if (edit.groupChanged())
    return EditResult::GroupChanged;
  return result;
}

void TriggerModel::import(const QString &xml)
{
  client.importWorldTriggers(xml);
}
