#pragma once
#include <QtWidgets/QTreeWidget>

class TreeBuilder;

class AbstractPrefsTree : public QWidget
{
  Q_OBJECT

public:
  explicit AbstractPrefsTree(QWidget *parent = nullptr);
  virtual ~AbstractPrefsTree() {};

protected:
  QVariant currentData() const;
  virtual bool addItem() = 0;
  virtual void buildTree(TreeBuilder &builder) = 0;
  virtual bool editItem(size_t index) = 0;
  virtual QString exportXml() const = 0;
  virtual void importXml(const QString &text) = 0;
  virtual void removeItem(size_t index) = 0;
  virtual void setItemButtonsEnabled(bool enabled) = 0;
  virtual QTreeWidget *tree() const = 0;

protected slots:
  void on_add_clicked();
  void on_edit_clicked();
  void on_export_xml_clicked();
  void on_import_xml_clicked();
  void on_remove_clicked();
  void on_tree_itemActivated(QTreeWidgetItem *item);
  void on_tree_itemDoubleClicked(QTreeWidgetItem *item);

private:
  void buildTree();
};