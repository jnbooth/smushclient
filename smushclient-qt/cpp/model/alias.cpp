#include "alias.h"
#include "../client.h"
#include "../ui/worlddetails/aliasedit.h"
#include "smushclient_qt/src/ffi/sender.cxxqt.h"
#include "smushclient_qt/src/ffi/sender_map.cxxqt.h"

// Public methods

AliasModel::AliasModel(SmushClient& client, QObject* parent)
  : AbstractSenderModel(client, SenderKind::Alias, parent)
{
}

// Protected overrides

bool
AliasModel::add(QWidget* parent)
{
  Alias alias;
  AliasEdit edit(alias, parent);
  if (edit.exec() == QDialog::Rejected) {
    return false;
  }
  return client.addWorldAlias(alias) == ApiCode::OK;
}

int
AliasModel::edit(size_t index, QWidget* parent)
{
  Alias alias(client, index);
  AliasEdit edit(alias, parent);
  if (edit.exec() == QDialog::Rejected) {
    return static_cast<int>(ReplaceSenderResult::Unchanged);
  }
  return client.replaceWorldAlias(index, alias);
}

ParseResult
AliasModel::import(const QString& xml)
{
  return client.importWorldAliases(xml);
}
