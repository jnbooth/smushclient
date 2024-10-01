#ifndef VIEWBUILDER_H
#define VIEWBUILDER_H

#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTreeWidget>

class TableBuilder
{
public:
  explicit TableBuilder(QTableWidget *table);

  void setRowCount(int rows) const;
  void startRow(const QString &data);
  void addColumn(const QString &text);
  void addColumn(bool value);

private:
  QTableWidget *table;
  QString yes;
  QString no;
  QString rowData;
  int row;
  int column;
};

class TreeBuilder
{
public:
  explicit TreeBuilder(QTreeWidget *tree);

  void startGroup(const QString &name);
  void startItem(size_t value);
  void addColumn(const QString &text);
  void addColumn(qlonglong value) { addColumn(QString::number(value)); }
  void addColumn(qulonglong value) { addColumn(QString::number(value)); }
  void addColumn(double value) { addColumn(QString::number(value, 'g', 2)); }

private:
  QTreeWidget *tree;
  QTreeWidgetItem *group;
  QTreeWidgetItem *item;
  int column;
};

#endif // VIEWBUILDER_H
