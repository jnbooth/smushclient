#include "abstractprefstree.h"
#include <QtGui/QClipboard>
#include <QtGui/QGuiApplication>
#include <QtWidgets/QErrorMessage>
#include "rust/cxx.h"

using std::vector;

constexpr size_t invalidIndex = SIZE_MAX;

// Public methods

AbstractPrefsTree::AbstractPrefsTree(QWidget *parent)
    : QWidget(parent),
      builder(new ModelBuilder(this)) {}

// Protected methods

void AbstractPrefsTree::buildTree()
{
  builder->clear();
  return buildTree(*builder);
}

// Protected slots

void AbstractPrefsTree::on_add_clicked()
{
  if (addItem())
    buildTree();
}

void AbstractPrefsTree::on_edit_clicked()
{
  const size_t index = clientIndex(tree()->currentIndex());
  if (index != invalidIndex && editItem(index))
    buildTree();
}

void AbstractPrefsTree::on_export_xml_clicked()
{
  try
  {
    QGuiApplication::clipboard()->setText(exportXml());
  }
  catch (const rust::Error &e)
  {
    QErrorMessage::qtHandler()->showMessage(QString::fromUtf8(e.what()));
  }
}

void AbstractPrefsTree::on_import_xml_clicked()
{
  try
  {
    importXml(QGuiApplication::clipboard()->text());
  }
  catch (const rust::Error &e)
  {
    QErrorMessage::qtHandler()->showMessage(QString::fromUtf8(e.what()));
  }
  buildTree();
}

void AbstractPrefsTree::on_remove_clicked()
{
  const QModelIndexList items = tree()->selectionModel()->selectedIndexes();
  vector<size_t> indexes;
  indexes.reserve(items.size());

  for (const QModelIndex &modelIndex : items)
    if (size_t index = clientIndex(modelIndex); index != invalidIndex)
      indexes.push_back(index);

  std::sort(indexes.rbegin(), indexes.rend());
  for (size_t index : indexes)
    removeItem(index);
  buildTree();
}

void AbstractPrefsTree::on_tree_activated(QModelIndex index)
{
  QStandardItem *item = model()->itemFromIndex(index);
  setItemButtonsEnabled(item && !item->hasChildren());
}

void AbstractPrefsTree::on_tree_doubleClicked(QModelIndex modelIndex)
{
  QItemSelectionModel *selection = tree()->selectionModel();
  if (selection->selectedIndexes().size() > 1)
  {
    selection->clear();
    selection->select(modelIndex, QItemSelectionModel::SelectionFlag::SelectCurrent);
  }
  if (size_t index = clientIndex(modelIndex); index != invalidIndex && editItem(index))
    buildTree();
}

// Private methods

size_t AbstractPrefsTree::clientIndex(QModelIndex index) const
{
  QStandardItem *item = model()->itemFromIndex(index);
  if (!item)
    return invalidIndex;
  QVariant data = item->data(Qt::UserRole);
  if (!data.canConvert<size_t>())
    return invalidIndex;
  return data.value<size_t>();
}
