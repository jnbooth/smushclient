#include "abstractprefstree.h"
#include <QtGui/QClipboard>
#include <QtGui/QGuiApplication>
#include <QtWidgets/QErrorMessage>
#include "rust/cxx.h"
#include "../../bridge/viewbuilder.h"

using std::vector;

// Public methods

AbstractPrefsTree::AbstractPrefsTree(QWidget *parent)
    : QWidget(parent) {}

// Protected methods

QVariant AbstractPrefsTree::currentData() const
{
  const QTreeWidgetItem *item = tree()->currentItem();
  if (!item)
    return QVariant();

  return item->data(0, Qt::UserRole);
}

// Protected slots

void AbstractPrefsTree::on_add_clicked()
{
  if (addItem())
    buildTree();
}

void AbstractPrefsTree::on_edit_clicked()
{
  const QVariant data = currentData();
  if (!data.canConvert<size_t>())
    return;
  const size_t index = data.value<size_t>();
  if (editItem(index))
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
  QList<QTreeWidgetItem *> items = tree()->selectedItems();
  vector<size_t> indexes;
  indexes.reserve(items.size());
  for (const QTreeWidgetItem *item : tree()->selectedItems())
  {
    QVariant data = item->data(0, Qt::UserRole);
    if (!data.canConvert<size_t>())
      continue;
    const size_t index = data.value<size_t>();
    indexes.push_back(index);
  }
  std::sort(indexes.rbegin(), indexes.rend());
  for (size_t index : indexes)
    removeItem(index);
  buildTree();
}

void AbstractPrefsTree::on_tree_itemActivated(QTreeWidgetItem *item)
{
  setItemButtonsEnabled(item && item->childCount() == 0);
}

void AbstractPrefsTree::on_tree_itemDoubleClicked(QTreeWidgetItem *item)
{
  QTreeWidget *treeWidget = item->treeWidget();
  if (treeWidget->selectedItems().size() > 1)
  {
    treeWidget->clearSelection();
    item->setSelected(true);
  }
  const QVariant data = item->data(0, Qt::UserRole);
  if (!data.canConvert<size_t>())
    return;
  const size_t index = data.value<size_t>();
  if (editItem(index))
    buildTree();
}

// Private methods

void AbstractPrefsTree::buildTree()
{
  QTreeWidget *treeWidget = tree();
  treeWidget->clear();
  TreeBuilder builder(treeWidget);
  buildTree(builder);
}
