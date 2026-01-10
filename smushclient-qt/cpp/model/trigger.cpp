#include "trigger.h"
#include "../client.h"
#include "../ui/worlddetails/triggeredit.h"
#include "smushclient_qt/src/ffi/sender.cxxqt.h"
#include "smushclient_qt/src/ffi/sender_map.cxxqt.h"

// Public methods

TriggerModel::TriggerModel(SmushClient& client, QObject* parent)
  : AbstractSenderModel(client, SenderType::Trigger, parent)
{
  client.stopTriggers();
}

// Public overrides

QString
TriggerModel::exportXml() const
{
  return client.exportWorldTriggers();
}

Qt::ItemFlags
TriggerModel::flags(const QModelIndex& index) const
{
  if (!index.constInternalPointer())
    return Qt::ItemFlag::ItemIsEnabled;

  return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled |
         Qt::ItemFlag::ItemNeverHasChildren | Qt::ItemFlag::ItemIsEditable;
}

// Protected overrides

bool
TriggerModel::add(QWidget* parent)
{
  Trigger trigger;
  TriggerEdit edit(trigger, parent);
  if (edit.exec() == QDialog::Rejected)
    return false;
  return client.addWorldTrigger(trigger) == ApiCode::OK;
}

int
TriggerModel::edit(size_t index, QWidget* parent)
{
  Trigger trigger(client, index);
  TriggerEdit edit(trigger, parent);
  if (edit.exec() == QDialog::Rejected)
    return (int)ReplaceSenderResult::Unchanged;
  return client.replaceWorldTrigger(index, trigger);
}

const std::array<QString, 4>&
TriggerModel::headers() const noexcept
{
  const static std::array<QString, 4> headers{
    tr("Group/Label"), tr("Sequence"), tr("Pattern"), tr("Text")
  };
  return headers;
}

void
TriggerModel::import(const QString& xml)
{
  client.importWorldTriggers(xml);
}
