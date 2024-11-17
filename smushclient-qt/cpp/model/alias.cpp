#include "alias.h"
#include "../ui/worlddetails/aliasedit.h"
#include "cxx-qt-gen/ffi.cxxqt.h"

AliasModel::AliasModel(SmushClient &client, QObject *parent)
    : AbstractSenderModel(client, SenderType::Alias, parent)
{
  setHeaders(tr("Group/Label"), tr("Sequence"), tr("Pattern"), tr("Text"));
}

int AliasModel::addItem(SmushClient &client, QWidget *parent)
{
  Alias alias;
  AliasEdit edit(alias, parent);
  if (edit.exec() == QDialog::Rejected)
    return -1;
  return client.addWorldAlias(alias);
}

int AliasModel::editItem(SmushClient &client, size_t index, QWidget *parent)
{
  Alias alias(&client, index);
  AliasEdit edit(alias, parent);
  if (edit.exec() == QDialog::Rejected)
    return -1;
  const int result = client.replaceWorldAlias(index, alias);
  if (result < 0)
    return -3;
  if (edit.groupChanged())
    return -2;
  return result;
}

QString AliasModel::exportXml(const SmushClient &client) const
{
  return client.exportWorldAliases();
}

void AliasModel::importXml(SmushClient &client, const QString &xml)
{
  client.importWorldAliases(xml);
}
