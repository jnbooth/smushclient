#include "../../fieldconnector.h"
#include "../../viewbuilder.h"
#include "aliases.h"
#include "ui_aliases.h"
#include "aliasedit.h"

PrefsAliases::PrefsAliases(World *world, QWidget *parent)
    : AbstractPrefsTree(parent), ui(new Ui::PrefsAliases), world(world), builder(nullptr)
{
  ui->setupUi(this);
  builder = TreeBuilder(ui->tree);
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

  world->addAlias(alias);
  buildTree();
}

void PrefsAliases::editItem(size_t index)
{
  Alias alias;
  world->getAlias(index, alias);
  AliasEdit edit(&alias, this);
  if (edit.exec() == QDialog::Rejected)
    return;

  world->replaceAlias(index, alias);
  buildTree();
}

void PrefsAliases::removeItem(size_t index)
{
  world->removeAlias(index);
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
  builder.clear();
  world->buildAliasTree(builder);
}
