#pragma once
#include <QtGui/QStandardItemModel>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QWidget>
#include "../../bridge/viewbuilder.h"

enum class ModelType;

class AbstractPrefsTree : public QWidget
{
  Q_OBJECT

public:
  explicit AbstractPrefsTree(ModelType modelType, QWidget *parent = nullptr);
  virtual ~AbstractPrefsTree();

protected:
  void buildTree();
  constexpr QStandardItemModel *model() const noexcept { return builder->model(); }
  void setHeaders(const QStringList &headers);
  void setTree(QTreeView *tree);

  virtual bool addItem() = 0;
  virtual void buildTree(ModelBuilder &builder) = 0;
  virtual bool editItem(size_t index) = 0;
  virtual QString exportXml() const = 0;
  virtual void importXml(const QString &text) = 0;
  virtual void removeItem(size_t index) = 0;
  virtual void setItemButtonsEnabled(bool enabled) = 0;

protected slots:
  void on_add_clicked();
  void on_edit_clicked();
  void on_export_xml_clicked();
  void on_import_xml_clicked();
  void on_remove_clicked();
  void on_tree_activated(QModelIndex index);
  void on_tree_doubleClicked(QModelIndex index);

private:
  ModelBuilder *builder;
  ModelType modelType;
  QTreeView *tree;

private:
  size_t clientIndex(QModelIndex index) const;
};
