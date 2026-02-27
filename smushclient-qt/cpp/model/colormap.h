#pragma once
#include <QtCore/QAbstractTableModel>
#include <QtGui/QColor>

class ColorMapModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  explicit ColorMapModel(QList<QPair<QColor, QColor>>& list,
                         QObject* parent = nullptr);
  bool setValue(const QModelIndex& index, const QColor& color);
  QColor value(const QModelIndex& index) const;
  int columnCount(const QModelIndex& index = QModelIndex()) const override
  {
    return index.isValid() ? 0 : numColumns;
  }
  QVariant data(const QModelIndex& index,
                int role = Qt::DisplayRole) const override;
  QVariant headerData(int section,
                      Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;
  QMap<int, QVariant> itemData(const QModelIndex& index) const override;
  bool insertRows(int row,
                  int count,
                  const QModelIndex& parent = QModelIndex()) override;
  bool removeRows(int row,
                  int count,
                  const QModelIndex& parent = QModelIndex()) override;
  int rowCount(const QModelIndex& index = QModelIndex()) const override
  {
    return index.isValid() ? 0 : static_cast<int>(list.size());
  }
  bool setData(const QModelIndex& index,
               const QVariant& value,
               int role = Qt::DisplayRole) override;

protected:
  static constexpr int numColumns = 3;

private:
  static const QIcon clearIcon;

  QList<QPair<QColor, QColor>>& list;
};
