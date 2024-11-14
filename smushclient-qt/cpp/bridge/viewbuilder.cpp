#include "viewbuilder.h"
#include <QtCore/QCoreApplication>
#include "../settings.h"

// Public methods

ModelBuilder::ModelBuilder(QObject *parent)
    : QObject(parent),
      headers(),
      row()
{
  items = new QStandardItemModel(this);
  no = tr("No");
  yes = tr("Yes");
  group = items->invisibleRootItem();
}

ModelBuilder::~ModelBuilder()
{
  qDeleteAll(row);
  row.clear();
}

bool ModelBuilder::getBool(QModelIndex index) const
{
  return items->itemFromIndex(index)->text() == yes;
}

void ModelBuilder::clear()
{
  qDeleteAll(row);
  items->clear();
  group = items->invisibleRootItem();
  row.clear();
  items->setHorizontalHeaderLabels(headers);
}

void ModelBuilder::setHeaders(const QStringList &newHeaders)
{
  headers = newHeaders;
  items->setHorizontalHeaderLabels(headers);
  for (int i = 0, end = headers.size(); i < end; ++i)
    items->setHeaderData(i, Qt::Orientation::Horizontal, i);
}

void ModelBuilder::startGroup(const QString &name)
{
  group = name.isEmpty() ? new QStandardItem(tr("(ungrouped)")) : new QStandardItem(name);
  items->appendRow(group);
}

void ModelBuilder::addColumn(const QString &text)
{
  row.push_back(new QStandardItem(text));
}

void ModelBuilder::finishRow(const QVariant &data)
{
  for (QStandardItem *item : row)
    item->setData(data);
  group->appendRow(row);
  row.clear();
}
