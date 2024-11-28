#include "abstractprefstree.h"
#include <QtGui/QClipboard>
#include <QtGui/QGuiApplication>
#include <QtWidgets/QErrorMessage>
#include <QtWidgets/QHeaderView>
#include "../../settings.h"
#include "../../model/sender.h"
#include "rust/cxx.h"

using std::vector;

// Public methods

AbstractPrefsTree::AbstractPrefsTree(AbstractSenderModel *model, QWidget *parent)
    : QWidget(parent),
      model(model)
{
  proxy = new QSortFilterProxyModel(this);
  proxy->setSourceModel(model);
  proxy->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  proxy->setRecursiveFilteringEnabled(true);
  proxy->setFilterKeyColumn(-1);
}

AbstractPrefsTree::~AbstractPrefsTree()
{
  Settings().setHeaderState(objectName(), tree->header()->saveState());
}

// Protected methods

void AbstractPrefsTree::setTree(QTreeView *newTree)
{
  tree = newTree;
  tree->header()->restoreState(Settings().getHeaderState(objectName()));
  if (filtering)
    tree->setModel(proxy);
  else
    tree->setModel(model);
  tree->expandAll();
  connect(tree->selectionModel(), &QItemSelectionModel::selectionChanged, this, &AbstractPrefsTree::onSelectionChanged);
}

// Protected slots

void AbstractPrefsTree::on_add_clicked()
{
  model->addItem(this);
}

void AbstractPrefsTree::on_edit_clicked()
{
  model->editItem(mapIndex(tree->currentIndex()), this);
  tree->expandAll();
}

void AbstractPrefsTree::on_export_xml_clicked()
{
  try
  {
    QGuiApplication::clipboard()->setText(model->exportXml());
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
    model->importXml(QGuiApplication::clipboard()->text());
  }
  catch (const rust::Error &e)
  {
    QErrorMessage::qtHandler()->showMessage(QString::fromUtf8(e.what()));
  }
}

void AbstractPrefsTree::on_remove_clicked()
{
  QItemSelection selection = tree->selectionModel()->selection();
  if (filtering)
    selection = proxy->mapSelectionToSource(selection);
  model->removeSelection(selection);
}

void AbstractPrefsTree::on_search_textChanged(const QString &text)
{
  proxy->setFilterFixedString(text);
  if (filtering == !text.isEmpty())
    return;

  filtering = !filtering;
  if (filtering)
    tree->setModel(proxy);
  else
    tree->setModel(model);
  tree->expandAll();
}

void AbstractPrefsTree::on_tree_doubleClicked(QModelIndex modelIndex)
{
  QItemSelectionModel *selection = tree->selectionModel();
  QItemSelection itemSelection;
  const int columns = model->columnCount(modelIndex);
  itemSelection.select(modelIndex.siblingAtColumn(0), modelIndex.siblingAtColumn(columns - 1));
  selection->select(itemSelection, QItemSelectionModel::SelectionFlag::Select);
  model->editItem(mapIndex(modelIndex), this);
  tree->expandAll();
}

// Private methods

QModelIndex AbstractPrefsTree::mapIndex(const QModelIndex &index) const
{
  return filtering ? proxy->mapToSource(index) : index;
}

// Private slots

void AbstractPrefsTree::onSelectionChanged()
{
  const QItemSelectionModel *selection = tree->selectionModel();
  if (!selection->hasSelection())
  {
    enableSingleButtons(false);
    enableMultiButtons(false);
    return;
  }
  enableSingleButtons(selection->selectedRows().length() == 1);
  enableMultiButtons(true);
}
