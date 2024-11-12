#include "aliases.h"
#include "aliasedit.h"
#include "ui_aliases.h"
#include "../../bridge/viewbuilder.h"
#include "../../fieldconnector.h"
#include "../../settings.h"

PrefsAliases::PrefsAliases(const World &world, SmushClient &client, QWidget *parent)
    : AbstractPrefsTree(ModelType::Alias, parent),
      ui(new Ui::PrefsAliases),
      client(client)
{
  ui->setupUi(this);
  setTree(ui->tree);
  setHeaders({tr("Group/Label"), tr("Sequence"), tr("Pattern"), tr("Text")});
  CONNECT_WORLD(EnableAliases);
  client.stopAliases();
  ui->tree->setModel(model());
  AbstractPrefsTree::buildTree();
}

PrefsAliases::~PrefsAliases()
{
  delete ui;
}

// Protected overrides

bool PrefsAliases::addItem()
{
  Alias alias;
  AliasEdit edit(&alias, this);
  if (edit.exec() == QDialog::Rejected)
    return false;

  client.addWorldAlias(alias);
  return true;
}

void PrefsAliases::buildTree(ModelBuilder &builder)
{
  ui->item_count->setNum((int)client.buildAliasesTree(builder, true));
}

bool PrefsAliases::editItem(size_t index)
{
  Alias alias(&client, index);
  AliasEdit edit(&alias, this);
  if (edit.exec() == QDialog::Rejected)
    return false;

  client.replaceWorldAlias(index, alias);
  return true;
}

QString PrefsAliases::exportXml() const
{
  return client.exportWorldAliases();
}

void PrefsAliases::importXml(const QString &xml)
{
  client.importWorldAliases(xml);
}

void PrefsAliases::removeItem(size_t index)
{
  client.removeWorldAlias(index);
}

void PrefsAliases::setItemButtonsEnabled(bool enabled)
{
  ui->edit->setEnabled(enabled);
}
