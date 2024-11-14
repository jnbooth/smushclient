#ifndef VIEWBUILDER_H
#define VIEWBUILDER_H

#include <QtGui/QStandardItemModel>

class ModelBuilder : public QObject
{
  Q_OBJECT

public:
  explicit ModelBuilder(QObject *parent = nullptr);
  ~ModelBuilder();

  inline constexpr QStandardItemModel *model() const noexcept { return items; }
  bool getBool(QModelIndex index) const;
  void clear();
  void setHeaders(const QStringList &headers);
  void startGroup(const QString &name);
  void addColumn(const QString &text);
  inline void addColumn(qlonglong value) { addColumn(QString::number(value)); }
  inline void addColumn(qulonglong value) { addColumn(QString::number(value)); }
  inline void addColumn(double value) { addColumn(QString::number(value, 'g', 2)); }
  inline void addColumn(bool value) { addColumn(value ? yes : no); }
  void finishRow(const QVariant &data);

private:
  QStandardItemModel *items;
  QStringList headers;
  QStandardItem *group;
  QList<QStandardItem *> row;
  QString no;
  QString yes;
};

#endif // VIEWBUILDER_H
