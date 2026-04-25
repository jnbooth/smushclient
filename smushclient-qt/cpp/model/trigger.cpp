#include "trigger.h"
#include "../client.h"
#include "../ui/worlddetails/triggeredit.h"
#include "smushclient_qt/src/ffi/sender.cxxqt.h"
#include "smushclient_qt/src/ffi/sender_map.cxxqt.h"

// Public methods

TriggerModel::TriggerModel(SmushClient& client, QObject* parent)
  : AbstractSenderModel(client, SenderKind::Trigger, parent)
{
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

ParseResult
TriggerModel::import(const QString& xml)
{
  return client.importWorldTriggers(xml);
}
