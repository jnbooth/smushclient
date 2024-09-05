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
  void addColumn(qint16 value) { addColumn(QString::number(value)); }

private:
  int column;
  QTreeWidget *tree;
  QTreeWidgetItem *group;
  QTreeWidgetItem *item;
};

#endif // VIEWBUILDER_H
