#include "viewbuilder.h"
#include <QtCore/QCoreApplication>

// Public methods

TableBuilder::TableBuilder(QTableWidget *table)
    : table(table),
      yes(QCoreApplication::translate("TableBuilder", "Yes")),
      no(QCoreApplication::translate("TableBuilder", "No")),
      rowData(),
      row(-1),
      column(0) {}

void TableBuilder::setRowCount(int rows) const
{
  table->setRowCount(rows);
}

void TableBuilder::startRow(const QVariant &data)
{
  ++row;
  column = 0;
  rowData = data;
}

void TableBuilder::addColumn(const QString &text)
{
  QTableWidgetItem *item = new QTableWidgetItem(text);
  item->setData(Qt::UserRole, rowData);
  table->setItem(row, column, item);
  ++column;
}

void TableBuilder::addColumn(bool value)
{
  addColumn(value ? yes : no);
}

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

void TreeBuilder::startRow(const QVariant &data)
{
  if (!group)
    startGroup(tree->tr("(ungrouped)"));

  item = new QTreeWidgetItem(group);
  item->setData(0, Qt::UserRole, data);
  column = 0;
}

void TreeBuilder::addColumn(const QString &text)
{
  if (!item)
    return;

  item->setText(column, text);
  ++column;
}
