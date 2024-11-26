#include "alias.h"
#include "../ui/worlddetails/aliasedit.h"
#include "smushclient_qt/src/bridge.cxxqt.h"

// Public methods

AliasModel::AliasModel(SmushClient &client, QObject *parent)
    : AbstractSenderModel(client, SenderType::Alias, parent)
{
  client.stopAliases();
}

Qt::ItemFlags AliasModel::flags(const QModelIndex &index) const
{
  if (!index.constInternalPointer())
    return Qt::ItemFlag::ItemIsEnabled;

  return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemNeverHasChildren | Qt::ItemIsEditable;
}

// Protected overrides

int AliasModel::addItem(SmushClient &client, QWidget *parent)
{
  Alias alias;
  AliasEdit edit(alias, parent);
  if (edit.exec() == QDialog::Rejected)
    return EditResult::Unchanged;
  return client.addWorldAlias(alias);
}

int AliasModel::editItem(SmushClient &client, size_t index, QWidget *parent)
{
  Alias alias(&client, index);
  AliasEdit edit(alias, parent);
  if (edit.exec() == QDialog::Rejected)
    return EditResult::Unchanged;
  const int result = client.replaceWorldAlias(index, alias);
  if (result == (int)SenderAccessResult::Unchanged)
    return EditResult::Unchanged;
  if (result < 0)
    return EditResult::Failed;
  if (edit.groupChanged())
    return EditResult::GroupChanged;
  return result;
}

QString AliasModel::exportXml(const SmushClient &client) const
{
  return client.exportWorldAliases();
}

const std::array<QString, 4> &AliasModel::headers() const noexcept
{
  const static std::array<QString, 4> headers{
      tr("Group/Label"),
      tr("Sequence"),
      tr("Pattern"),
      tr("Text")};
  return headers;
}

void AliasModel::importXml(SmushClient &client, const QString &xml)
{
  client.importWorldAliases(xml);
}
