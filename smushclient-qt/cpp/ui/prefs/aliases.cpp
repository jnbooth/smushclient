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
  buildTree();
}

PrefsAliases::~PrefsAliases()
{
  delete ui;
}

// Protected overrides

void PrefsAliases::addItem()
{
  Alias alias;
  AliasEdit edit(&alias, this);
  if (edit.exec() == QDialog::Rejected)
    return;

  world.addAlias(alias);
  buildTree();
}

void PrefsAliases::editItem(size_t index)
{
  Alias alias(&world, index);
  AliasEdit edit(&alias, this);
  if (edit.exec() == QDialog::Rejected)
    return;

  world.replaceAlias(index, alias);
  buildTree();
}

QString PrefsAliases::exportXml() const
{
  return world.exportAliases();
}

QString PrefsAliases::importXml(const QString &xml)
{
  const QString result = world.importAliases(xml);
  if (result.isEmpty())
    buildTree();
  return result;
}

void PrefsAliases::removeItem(size_t index)
{
  world.removeAlias(index);
  buildTree();
}

void PrefsAliases::setItemButtonsEnabled(bool enabled)
{
  ui->edit->setEnabled(enabled);
}

QTreeWidget *PrefsAliases::tree() const
{
  return ui->tree;
}

// Private methods

void PrefsAliases::buildTree()
{
  ui->tree->clear();
  TreeBuilder builder(ui->tree);
  world.buildAliasTree(builder);
}
