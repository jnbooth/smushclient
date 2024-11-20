#pragma once
#include <QtCore/QSortFilterProxyModel>
#include <QtGui/QStandardItemModel>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QWidget>

class SmushClient;
class AbstractSenderModel;

class AbstractPrefsTree : public QWidget
{
  Q_OBJECT

public:
  explicit AbstractPrefsTree(AbstractSenderModel *model, QWidget *parent = nullptr);
  virtual ~AbstractPrefsTree();

protected:
  virtual void enableSingleButtons(bool enabled) = 0;
  virtual void enableMultiButtons(bool enabled) = 0;
  void setTree(QTreeView *tree);

protected slots:
  void on_add_clicked();
  void on_edit_clicked();
  void on_export_xml_clicked();
  void on_import_xml_clicked();
  void on_remove_clicked();
  void on_search_textChanged(const QString &text);
  void on_tree_doubleClicked(QModelIndex index);

private:
  bool filtering;
  AbstractSenderModel *model;
  QSortFilterProxyModel *proxy;
  QTreeView *tree;

private:
  QModelIndex mapIndex(const QModelIndex &index) const;

private slots:
  void onSelectionChanged();
};
