#include "colormap.h"
#include <QtGui/QIcon>

using std::array;

// Private static variables

const QIcon ColorMapModel::clearIcon =
  QIcon::fromTheme(QIcon::ThemeIcon::EditClear);

// Public methods

ColorMapModel::ColorMapModel(QList<QPair<QColor, QColor>>& list,
                             QObject* parent)
  : QAbstractTableModel(parent)
  , list(list)
{
}

bool
ColorMapModel::setValue(const QModelIndex& index, const QColor& color)
{
  if (!index.isValid()) {
    return false;
  }
  switch (index.column()) {
    case 0:
      list[index.row()].first = color;
      emit dataChanged(index, index, { Qt::BackgroundRole });
      return true;
    case 1:
      list[index.row()].second = color;
      emit dataChanged(index, index, { Qt::BackgroundRole });
      return true;
    default:
      return false;
  }
}

QColor
ColorMapModel::value(const QModelIndex& index) const
{
  if (!index.isValid()) {
    return QColor();
  }
  switch (index.column()) {
    case 0:
      return list[index.row()].first;
    case 1:
      return list[index.row()].second;
    default:
      return QColor();
  }
}

// Public overrides

QVariant
ColorMapModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid()) {
    return QVariant();
  }
  if (role == Qt::DecorationRole && index.column() == 2) {
    return clearIcon;
  }
  if (role != Qt::BackgroundRole) {
    return QVariant();
  }
  switch (index.column()) {
    case 0:
      return list[index.row()].first;
    case 1:
      return list[index.row()].second;
    default:
      return QVariant();
  }
}

QVariant
ColorMapModel::headerData(int section,
                          Qt::Orientation orientation,
                          int role) const
{
  if (orientation != Qt::Orientation::Horizontal || role != Qt::DisplayRole) {
    return QVariant();
  }

  const static array<QString, numColumns> headers{ tr("From"),
                                                   tr("To"),
                                                   QString() };

  return headers.at(section);
}

QMap<int, QVariant>
ColorMapModel::itemData(const QModelIndex& index) const
{
  QMap<int, QVariant> map;
  if (index.column() == 2) {
    map.insert(Qt::DecorationRole, clearIcon);
  } else {
    map.insert(Qt::BackgroundRole, value(index));
  }
  return map;
}

bool
ColorMapModel::insertRows(int row, int count, const QModelIndex& parent)
{
  if (row < 0 || count <= 0 || row > list.size() || parent.isValid()) {
    return false;
  }
  emit layoutAboutToBeChanged();
  beginInsertRows(parent, row, row + count - 1);
  for (int i = 0; i < count; ++i) {
    list.insert(i, { Qt::GlobalColor::gray, Qt::GlobalColor::gray });
  }
  endInsertRows();
  emit layoutChanged();
  return true;
}

bool
ColorMapModel::removeRows(int row, int count, const QModelIndex& parent)
{
  if (row < 0 || count <= 0 || row + count > list.size() || parent.isValid()) {
    return false;
  }

  emit layoutAboutToBeChanged();
  beginRemoveRows(parent, row, row + count - 1);
  list.remove(row, count);
  endRemoveRows();
  emit layoutChanged();
  return true;
}

bool
ColorMapModel::setData(const QModelIndex& index,
                       const QVariant& value,
                       int role)
{
  if (role != Qt::BackgroundRole) {
    return false;
  }
  return setValue(index, value.value<QColor>());
}
