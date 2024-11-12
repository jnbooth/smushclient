#ifndef VIEWBUILDER_H
#define VIEWBUILDER_H

#include <QtGui/QStandardItemModel>

class ModelBuilder : public QObject
{
public:
  explicit ModelBuilder(QObject *parent = nullptr);
  ~ModelBuilder();

  inline constexpr QStandardItemModel *model() const noexcept { return items; }
  void clear();
  void startGroup(const QString &name);
  void addColumn(const QString &text);
  inline void addColumn(qlonglong value) { addColumn(QString::number(value)); }
  inline void addColumn(qulonglong value) { addColumn(QString::number(value)); }
  inline void addColumn(double value) { addColumn(QString::number(value, 'g', 2)); }
  inline void addColumn(bool value) { addColumn(value ? yes : no); }
  void finishRow(const QVariant &data);
  void startReplacement(const QModelIndex &index);

private:
  QStandardItemModel *items;
  QStandardItem *group;
  QList<QStandardItem *> row;
  QModelIndex replacing;
  QString no;
  QString yes;
};

#endif // VIEWBUILDER_H
