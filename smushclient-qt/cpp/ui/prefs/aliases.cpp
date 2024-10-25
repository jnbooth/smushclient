#include "aliases.h"
#include "aliasedit.h"
#include "ui_aliases.h"
#include "../../bridge/viewbuilder.h"
#include "../../fieldconnector.h"

PrefsAliases::PrefsAliases(World &world, QWidget *parent)
    : AbstractPrefsTree(parent),
      ui(new Ui::PrefsAliases),
      world(world)
{
  ui->setupUi(this);
  CONNECT_WORLD(EnableAliases);
  ui->tree->clear();
  TreeBuilder builder(ui->tree);
  world.buildAliasTree(builder);
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

  world.addAlias(alias);
  return true;
}

void PrefsAliases::buildTree(TreeBuilder &builder)
{
  world.buildAliasTree(builder);
}

bool PrefsAliases::editItem(size_t index)
{
  Alias alias(&world, index);
  AliasEdit edit(&alias, this);
  if (edit.exec() == QDialog::Rejected)
    return false;

  world.replaceAlias(index, alias);
  return true;
}

QString PrefsAliases::exportXml() const
{
  return world.exportAliases();
}

QString PrefsAliases::importXml(const QString &xml)
{
  return world.importAliases(xml);
}

void PrefsAliases::removeItem(size_t index)
{
  world.removeAlias(index);
}

void PrefsAliases::setItemButtonsEnabled(bool enabled)
{
  ui->edit->setEnabled(enabled);
}

QTreeWidget *PrefsAliases::tree() const
{
  return ui->tree;
}
