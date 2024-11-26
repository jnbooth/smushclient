#include "trigger.h"
#include "../ui/worlddetails/triggeredit.h"
#include "smushclient_qt/src/bridge.cxxqt.h"

// Public methods

TriggerModel::TriggerModel(SmushClient &client, QObject *parent)
    : AbstractSenderModel(client, SenderType::Trigger, parent)
{
  client.stopTriggers();
}

Qt::ItemFlags TriggerModel::flags(const QModelIndex &index) const
{
  if (!index.constInternalPointer())
    return Qt::ItemFlag::ItemIsEnabled;

  return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemNeverHasChildren | Qt::ItemIsEditable;
}

// Protected overrides

int TriggerModel::addItem(SmushClient &client, QWidget *parent)
{
  Trigger trigger;
  TriggerEdit edit(trigger, parent);
  if (edit.exec() == QDialog::Rejected)
    return EditResult::Unchanged;
  return client.addWorldTrigger(trigger);
}

int TriggerModel::editItem(SmushClient &client, size_t index, QWidget *parent)
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

QString TriggerModel::exportXml(const SmushClient &client) const
{
  return client.exportWorldTriggers();
}

const std::array<QString, 4> &TriggerModel::headers() const noexcept
{
  const static std::array<QString, 4> headers{
      tr("Group/Label"),
      tr("Sequence"),
      tr("Pattern"),
      tr("Text")};
  return headers;
}

void TriggerModel::importXml(SmushClient &client, const QString &xml)
{
  client.importWorldTriggers(xml);
}
