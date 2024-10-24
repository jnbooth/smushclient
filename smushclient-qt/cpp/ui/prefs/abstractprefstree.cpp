#include "abstractprefstree.h"
#include <QtGui/QClipboard>
#include <QtGui/QGuiApplication>
#include <QtWidgets/QErrorMessage>

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

void AbstractPrefsTree::on_export_xml_clicked()
{
  QGuiApplication::clipboard()->setText(exportXml());
}

void AbstractPrefsTree::on_import_xml_clicked()
{
  QString error = importXml(QGuiApplication::clipboard()->text());
  if (error.isEmpty())
    return;

  QErrorMessage::qtHandler()->showMessage(error);
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
  setItemButtonsEnabled(item && item->childCount() == 0);
}

void AbstractPrefsTree::on_tree_itemDoubleClicked(QTreeWidgetItem *item)
{
  const QVariant data = item->data(0, Qt::UserRole);
  if (!data.canConvert<size_t>())
    return;
  const size_t index = data.value<size_t>();
  editItem(index);
}
