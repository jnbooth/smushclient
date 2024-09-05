#ifndef ABSTRACTPREFSTREE_H
#define ABSTRACTPREFSTREE_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QTreeWidgetItem>

class AbstractPrefsTree : public QWidget
{
  Q_OBJECT

public:
  explicit AbstractPrefsTree(QWidget *parent = nullptr);
  virtual ~AbstractPrefsTree() {};

protected:
  QVariant currentData() const;
  virtual void addItem() = 0;
  virtual void editItem(size_t index) = 0;
  virtual void removeItem(size_t index) = 0;
  virtual void setItemButtonsEnabled(bool enabled) = 0;
  virtual QTreeWidget *tree() const = 0;

protected slots:
  void on_add_clicked(bool checked);
  void on_edit_clicked(bool checked);
  void on_remove_clicked(bool checked);
  void on_tree_itemActivated(QTreeWidgetItem *item, int column);
  void on_tree_itemDoubleClicked(QTreeWidgetItem *item, int column);
};

#endif // ABSTRACTPREFSTREE_H
