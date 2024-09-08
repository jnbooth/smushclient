#include "abstractprefstree.h"

AbstractPrefsTree::AbstractPrefsTree(QWidget *parent)
    : QWidget(parent) {}

QVariant AbstractPrefsTree::currentData() const
{
  const QTreeWidgetItem *item = tree()->currentItem();
  if (item == nullptr)
    return QVariant();

  return item->data(0, Qt::UserRole);
}

void AbstractPrefsTree::on_add_clicked()
{
  addItem();
}

void AbstractPrefsTree::on_edit_clicked()
{
  const QVariant data = currentData();
  if (!data.canConvert<size_t>())
    return;
  const size_t index = data.value<size_t>();
  editItem(index);
}

void AbstractPrefsTree::on_remove_clicked()
{
  const QVariant data = currentData();
  if (!data.canConvert<size_t>())
    return;
  const size_t index = data.value<size_t>();
  removeItem(index);
}

void AbstractPrefsTree::on_tree_itemActivated(QTreeWidgetItem *item)
{
  setItemButtonsEnabled(item != nullptr && item->childCount() == 0);
}

void AbstractPrefsTree::on_tree_itemDoubleClicked(QTreeWidgetItem *item)
{
  const QVariant data = item->data(0, Qt::UserRole);
  if (!data.canConvert<size_t>())
    return;
  const size_t index = data.value<size_t>();
  editItem(index);
}
