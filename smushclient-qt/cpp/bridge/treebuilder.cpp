#include "treebuilder.h"

TreeBuilder::TreeBuilder(QTreeWidget *tree)
    : tree(tree),
      group(nullptr),
      item(nullptr),
      column(0) {}

void TreeBuilder::startGroup(const QString &name)
{
  group = new QTreeWidgetItem(tree);
  group->setText(0, name);
  group->setExpanded(true);
}

void TreeBuilder::startItem(size_t value)
{
  if (group == nullptr)
    startGroup(tree->tr("(ungrouped)"));

  item = new QTreeWidgetItem(group);
  item->setData(0, Qt::UserRole, qulonglong(value));
  column = 0;
}

void TreeBuilder::addColumn(const QString &text)
{
  if (item == nullptr)
    return;

  item->setText(column, text);
  column += 1;
}
