#include "alias.h"
#include "../client.h"
#include "../ui/worlddetails/aliasedit.h"
#include "smushclient_qt/src/ffi/sender.cxxqt.h"
#include "smushclient_qt/src/ffi/sender_map.cxxqt.h"

using std::array;

// Public methods

AliasModel::AliasModel(SmushClient& client, QObject* parent)
  : AbstractSenderModel(client, SenderType::Alias, parent)
{
  client.stopAliases();
}

// Public overrides

QString
AliasModel::exportXml() const
{
  return client.exportWorldAliases();
}

Qt::ItemFlags
AliasModel::flags(const QModelIndex& index) const
{
  if (index.constInternalPointer() == nullptr) {
    return Qt::ItemFlag::ItemIsEnabled;
  }

  return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled |
         Qt::ItemFlag::ItemNeverHasChildren | Qt::ItemFlag::ItemIsEditable;
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

const array<QString, 4>&
AliasModel::headers() const noexcept
{
  const static array<QString, 4> headers{
    tr("Group/Label"), tr("Sequence"), tr("Pattern"), tr("Text")
  };
  return headers;
}

RegexParse
AliasModel::import(const QString& xml)
{
  return client.importWorldAliases(xml);
}
