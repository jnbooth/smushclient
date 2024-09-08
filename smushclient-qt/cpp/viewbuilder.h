#ifndef VIEWBUILDER_H
#define VIEWBUILDER_H

#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QTreeWidgetItem>

class TreeBuilder
{
public:
  explicit TreeBuilder(QTreeWidget *tree);

  void clear();
  void startGroup(const QString &name);
  void startItem(size_t value);
  void addColumn(const QString &text);
  void addColumn(qlonglong value) { addColumn(QString::number(value)); }
  void addColumn(qulonglong value) { addColumn(QString::number(value)); }
  void addColumn(double value) { addColumn(QString::number(value, 'g', 2)); }

private:
  int column;
  QTreeWidget *tree;
  QTreeWidgetItem *group;
  QTreeWidgetItem *item;
};

#endif // VIEWBUILDER_H