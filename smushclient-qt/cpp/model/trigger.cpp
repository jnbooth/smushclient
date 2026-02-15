#include "trigger.h"
#include "../client.h"
#include "../ui/worlddetails/triggeredit.h"
#include "smushclient_qt/src/ffi/sender.cxxqt.h"
#include "smushclient_qt/src/ffi/sender_map.cxxqt.h"

using std::array;

// Public methods

TriggerModel::TriggerModel(SmushClient& client, QObject* parent)
  : AbstractSenderModel(client, SenderType::Trigger, parent)
{
  client.stopSenders(SenderKind::Trigger);
}

// Public overrides

QString
TriggerModel::exportXml() const
{
  return client.exportWorldSenders(SenderKind::Trigger);
}

Qt::ItemFlags
TriggerModel::flags(const QModelIndex& index) const
{
  if (hasChildren(index)) {
    return Qt::ItemFlag::ItemIsEnabled;
  }

  return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled |
         Qt::ItemFlag::ItemNeverHasChildren | Qt::ItemFlag::ItemIsEditable;
}

// Protected overrides

bool
TriggerModel::add(QWidget* parent)
{
  Trigger trigger;
  TriggerEdit edit(trigger, parent);
  if (edit.exec() == QDialog::Rejected) {
    return false;
  }
  return client.addWorldTrigger(trigger) == ApiCode::OK;
}

int
TriggerModel::edit(size_t index, QWidget* parent)
{
  Trigger trigger(client, index);
  TriggerEdit edit(trigger, parent);
  if (edit.exec() == QDialog::Rejected) {
    return static_cast<int>(ReplaceSenderResult::Unchanged);
  }
  return client.replaceWorldTrigger(index, trigger);
}

const array<QString, 4>&
TriggerModel::headers() const noexcept
{
  const static array<QString, 4> headers{
    tr("Group/Label"), tr("Sequence"), tr("Pattern"), tr("Text")
  };
  return headers;
}

RegexParse
TriggerModel::import(const QString& xml)
{
  return client.importWorldTriggers(xml);
}
