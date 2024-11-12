#include "viewbuilder.h"
#include <QtCore/QCoreApplication>

// Public methods

ModelBuilder::ModelBuilder(QObject *parent)
    : QObject(parent),
      row(),
      replacing(),
      no(tr("No")),
      yes(tr("Yes"))
{
  items = new QStandardItemModel(this);
  group = items->invisibleRootItem();
}

ModelBuilder::~ModelBuilder()
{
  qDeleteAll(row);
  row.clear();
}

void ModelBuilder::clear()
{
  qDeleteAll(row);
  items->clear();
  group = items->invisibleRootItem();
  row.clear();
}

void ModelBuilder::startGroup(const QString &name)
{
  group = name.isEmpty() ? new QStandardItem(tr("(ungrouped)")) : new QStandardItem(name);
  items->appendRow(group);
}

void ModelBuilder::addColumn(const QString &text)
{
  if (!replacing.isValid())
  {
    row.push_back(new QStandardItem(text));
    return;
  }
  QStandardItem *item = items->itemFromIndex(replacing);
  if (!item)
  {
    replacing = QModelIndex();
    return;
  }
  item->setText(text);
  replacing = replacing.siblingAtColumn(replacing.column() + 1);
  return;
}

void ModelBuilder::finishRow(const QVariant &data)
{
  for (QStandardItem *item : row)
    item->setData(data);
  group->appendRow(row);
  row.clear();
  replacing = QModelIndex();
}

void ModelBuilder::startReplacement(const QModelIndex &index)
{
  replacing = index;
}
