#include "colormap.h"
#include <QtGui/QIcon>

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
  QColor* currentColor = get(index);
  if (currentColor == nullptr) {
    return false;
  }
  *currentColor = color;
  emit dataChanged(index, index, { Qt::BackgroundRole });
  return true;
}

QColor
ColorMapModel::value(const QModelIndex& index) const
{
  const QColor* color = get(index);
  return color == nullptr ? QColor() : *color;
}

// Public overrides

QVariant
ColorMapModel::data(const QModelIndex& index, int role) const
{
  switch (role) {
    case Qt::DecorationRole:
      return index.column() == 2 ? clearIcon : QVariant();
    case Qt::BackgroundRole:
      if (const QColor* color = get(index); color) {
        return *color;
      }
    default:
      return QVariant();
  };
}

QVariant
ColorMapModel::headerData(int section,
                          Qt::Orientation orientation,
                          int role) const
{
  static const std::array<QString, numColumns> headers{ tr("From"),
                                                        tr("To"),
                                                        QString() };

  if (orientation != Qt::Orientation::Horizontal || role != Qt::DisplayRole) {
    return QVariant();
  }

  return headers.at(section);
}

QMap<int, QVariant>
ColorMapModel::itemData(const QModelIndex& index) const
{
  if (const QColor* color = get(index); color) {
    return { { Qt::BackgroundRole, *color } };
  }
  if (index.column() == 2) {
    return { { Qt::DecorationRole, clearIcon } };
  }
  return {};
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

// Private methods

QColor*
ColorMapModel::get(const QModelIndex& index) const
{
  if (!index.isValid()) {
    return nullptr;
  }
  auto& [fore, back] = list[index.row()];
  switch (index.column()) {
    case 0:
      return &fore;
    case 1:
      return &back;
    default:
      return nullptr;
  }
}
