#include "aliases.h"
#include "aliasedit.h"
#include "ui_aliases.h"
#include "../../bridge/viewbuilder.h"
#include "../../fieldconnector.h"

PrefsAliases::PrefsAliases(const World &world, SmushClient &client, QWidget *parent)
    : AbstractPrefsTree(parent),
      ui(new Ui::PrefsAliases),
      client(client)
{
  ui->setupUi(this);
  CONNECT_WORLD(EnableAliases);
  client.stopAliases();
  ui->tree->clear();
  TreeBuilder builder(ui->tree);
  client.buildAliasesTree(builder);
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

void PrefsAliases::buildTree(TreeBuilder &builder)
{
  client.buildAliasesTree(builder);
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

QTreeWidget *PrefsAliases::tree() const
{
  return ui->tree;
}
