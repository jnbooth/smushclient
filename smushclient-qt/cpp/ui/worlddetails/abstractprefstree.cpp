#include "abstractprefstree.h"
#include <QtGui/QClipboard>
#include <QtGui/QGuiApplication>
#include <QtWidgets/QErrorMessage>
#include <QtWidgets/QHeaderView>
#include "../../settings.h"
#include "rust/cxx.h"

using std::vector;

constexpr size_t invalidIndex = SIZE_MAX;

// Public methods

AbstractPrefsTree::AbstractPrefsTree(ModelType modelType, QWidget *parent)
    : QWidget(parent),
      modelType(modelType),
      tree(nullptr)
{
  builder = new ModelBuilder(this);
  proxy = new QSortFilterProxyModel(this);
  proxy->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  proxy->setSourceModel(builder->model());
  proxy->setRecursiveFilteringEnabled(true);
  proxy->setFilterKeyColumn(-1);
}

AbstractPrefsTree::~AbstractPrefsTree()
{
  Settings().setHeaderState(modelType, tree->header()->saveState());
}

// Protected methods

void AbstractPrefsTree::buildTree()
{
  QHeaderView *header = tree->header();
  const QByteArray headerState =
      builder->model()->rowCount() == 0 ? Settings().headerState(modelType) : header->saveState();

  builder->clear();
  buildTree(*builder);
  tree->expandAll();

  header->restoreState(headerState);
}

void AbstractPrefsTree::setHeaders(const QStringList &newHeaders)
{
  builder->setHeaders(newHeaders);
}

void AbstractPrefsTree::setTree(QTreeView *newTree)
{
  tree = newTree;
  tree->setModel(proxy);
}

// Protected slots

void AbstractPrefsTree::on_add_clicked()
{
  if (addItem())
    buildTree();
}

void AbstractPrefsTree::on_edit_clicked()
{
  const size_t index = clientIndex(tree->currentIndex());
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
  const QModelIndexList items = tree->selectionModel()->selectedIndexes();
  vector<size_t> indexes;
  indexes.reserve(items.size());

  for (const QModelIndex &modelIndex : items)
    if (size_t index = clientIndex(modelIndex); index != invalidIndex)
      indexes.push_back(index);

  std::sort(indexes.rbegin(), indexes.rend());
  size_t lastIndex = -1;
  for (size_t index : indexes)
  {
    if (index == lastIndex)
      continue;
    removeItem(index);
    lastIndex = index;
  }
  buildTree();
}

void AbstractPrefsTree::on_search_textChanged(const QString &text)
{
  proxy->setFilterFixedString(text);
  tree->expandAll();
}

void AbstractPrefsTree::on_tree_activated(QModelIndex index)
{
  setItemButtonsEnabled(!proxy->hasChildren(index));
}

void AbstractPrefsTree::on_tree_doubleClicked(QModelIndex modelIndex)
{
  QItemSelectionModel *selection = tree->selectionModel();
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
  const QVariant data = proxy->data(index, Qt::UserRole + 1);
  if (!data.canConvert<size_t>())
    return invalidIndex;
  return data.value<size_t>();
}
