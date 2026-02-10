#pragma once
#include <QtCore/QSortFilterProxyModel>
#include <QtWidgets/QTreeView>

class SmushClient;
class AbstractSenderModel;

class AbstractPrefsTree : public QWidget
{
  Q_OBJECT

public:
  explicit AbstractPrefsTree(AbstractSenderModel& model,
                             QWidget* parent = nullptr);
  ~AbstractPrefsTree() override;

protected:
  virtual void enableSingleButtons(bool enabled) = 0;
  virtual void enableMultiButtons(bool enabled) = 0;
  void setTree(QTreeView* tree);

protected slots:
  void on_add_clicked();
  void on_edit_clicked();
  void on_export_xml_clicked();
  void on_import_xml_clicked();
  void on_remove_clicked();
  void on_search_textChanged(const QString& text);
  void on_tree_doubleClicked(QModelIndex index);

private:
  QModelIndex mapIndex(const QModelIndex& index) const;
  QString settingsKey() const;

private slots:
  void onSelectionChanged();

private:
  bool filtering = false;
  AbstractSenderModel& model;
  QSortFilterProxyModel* proxy;
  QTreeView* tree = nullptr;
};
