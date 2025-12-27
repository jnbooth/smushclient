#include "plugin.h"
#include "../client.h"
#include "smushclient_qt/src/ffi/plugin_details.cxxqt.h"

using std::nullopt;
using std::optional;

// Public methods

PluginModel::PluginModel(SmushClient& client, QObject* parent)
  : QAbstractItemModel(parent)
  , client(client)
  , pluginCount((int)client.pluginsLen() - 1)
  , worldIndex((int)client.worldPluginIndex())
{
}

bool
PluginModel::addPlugin(const QString& filePath)
{
  int pluginIndex;
  try {
    pluginIndex = (int)client.addPlugin(filePath);
  } catch (rust::Error e) {
    const QString error = QString::fromUtf8(e.what());
    emit clientError(error);
    return false;
  }
  emit layoutAboutToBeChanged();
  if (pluginIndex <= worldIndex)
    worldIndex += 1;
  const int row = pluginIndexToRow(pluginIndex);
  beginInsertRows(createIndex(0, 0), row, row);
  pluginCount += 1;
  endInsertRows();
  if (pluginIndex + 1 != pluginCount)
    emit pluginOrderChanged();
  emit layoutChanged();
  return true;
}

PluginDetails
PluginModel::pluginDetails(const QModelIndex& index) const
{
  return PluginDetails(client, pluginId(index));
}

QString
PluginModel::pluginId(const QModelIndex& index) const
{
  return client.pluginId(pluginIndex(index.row()));
}

bool
PluginModel::reinstall(const QModelIndex& index)
{
  if (!isValidIndex(index))
    return false;
  const int row = index.row();
  const size_t oldIndex = pluginIndex(row);
  size_t newIndex;
  try {
    newIndex = client.reinstallPlugin(oldIndex);
  } catch (rust::Error e) {
    const QString error = QString::fromUtf8(e.what());
    emit clientError(error);
    return false;
  }
  if (oldIndex == newIndex) {
    emit dataChanged(createIndex(row, 0),
                     createIndex(row, numColumns - 1),
                     { Qt::DisplayRole });
    emit pluginScriptChanged(newIndex);
    return true;
  }

  emit layoutAboutToBeChanged(
    {}, QAbstractItemModel::LayoutChangeHint::VerticalSortHint);

  const size_t worldPluginIndex = worldIndex;
  if (oldIndex > worldPluginIndex && newIndex <= worldPluginIndex)
    worldIndex += 1;
  else if (oldIndex < worldPluginIndex && newIndex >= worldPluginIndex)
    worldIndex -= 1;
  const int newRow = pluginIndexToRow((int)newIndex);
  const QModelIndex parent = createIndex(0, 0);

  beginMoveRows(parent, row, row, parent, newRow);
  endMoveRows();

  emit pluginOrderChanged();
  emit dataChanged(createIndex(newRow, 0),
                   createIndex(newRow, numColumns - 1),
                   { Qt::DisplayRole });
  emit pluginScriptChanged(newIndex);

  emit layoutChanged({},
                     QAbstractItemModel::LayoutChangeHint::VerticalSortHint);
  return true;
}

// Public overrides

QVariant
PluginModel::data(const QModelIndex& index, int role) const
{
  if (!isValidIndex(index))
    return QVariant();

  switch (role) {
    case Qt::DisplayRole:
      return client.pluginModelText(pluginIndex(index.row()), index.column());
    case Qt::CheckStateRole:
      if (index.column() != 4)
        return QVariant();
      return client.pluginEnabled(pluginIndex(index.row()))
               ? Qt::CheckState::Checked
               : Qt::CheckState::Unchecked;
    case Qt::InitialSortOrderRole:
      return (int)pluginIndex(index.row());
    default:
      return QVariant();
  }
}

Qt::ItemFlags
PluginModel::flags(const QModelIndex& index) const
{
  if (!isValidIndex(index))
    return Qt::ItemFlag::NoItemFlags;

  const Qt::ItemFlags flags = Qt::ItemFlag::ItemIsSelectable |
                              Qt::ItemFlag::ItemIsEnabled |
                              Qt::ItemFlag::ItemNeverHasChildren;

  if (index.column() != 4)
    return flags;

  return flags | Qt::ItemFlag::ItemIsUserCheckable;
}

bool
PluginModel::hasChildren(const QModelIndex&) const
{
  return false;
}

QVariant
PluginModel::headerData(int section,
                        Qt::Orientation orientation,
                        int role) const
{
  if (orientation != Qt::Orientation::Horizontal || !isValidColumn(section))
    return QVariant();

  const static std::array<QString, numColumns> headers{
    tr("Name"), tr("Purpose"), tr("Author"),
    tr("Path"), tr("Enabled"), tr("Version")
  };

  switch (role) {
    case Qt::DisplayRole:
      return headers[section];
    default:
      return QVariant();
  }
}

QModelIndex
PluginModel::index(int row, int column, const QModelIndex& parent) const
{
  if (!isValidColumn(column) || (row < 0) || (row >= rowCount(parent)))
    return QModelIndex();

  return createIndex(row, column);
}

QMap<int, QVariant>
PluginModel::itemData(const QModelIndex& index) const
{
  if (!isValidIndex(index))
    return QMap<int, QVariant>();

  QMap<int, QVariant> map;
  map.insert(Qt::DisplayRole, data(index, Qt::DisplayRole));
  map.insert(Qt::CheckStateRole, data(index, Qt::CheckStateRole));
  map.insert(Qt::InitialSortOrderRole, data(index, Qt::InitialSortOrderRole));
  return map;
}

QModelIndex
PluginModel::parent(const QModelIndex&) const
{
  return QModelIndex();
}

bool
PluginModel::removeRows(int row, int count, const QModelIndex& parent)
{
  if (row < 0 || count <= 0 || row + count > pluginCount ||
      parent.constInternalPointer())
    return false;

  emit layoutAboutToBeChanged();
  beginRemoveRows(parent, row, row + count - 1);
  bool succeeded = true;
  int i = 0;
  for (; i < count && succeeded; ++i) {
    if (row >= worldIndex)
      succeeded = client.removePlugin(row + 1);
    else {
      succeeded = client.removePlugin(row);
      worldIndex -= 1;
    }
  }
  pluginCount -= i;
  endRemoveRows();
  emit pluginOrderChanged();
  emit layoutChanged();
  return succeeded;
}

int
PluginModel::rowCount(const QModelIndex& index) const
{
  return index.constInternalPointer() ? 0 : pluginCount;
}

bool
PluginModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (role != Qt::CheckStateRole || index.column() != 4 || index.row() < 0 ||
      index.row() >= pluginCount || index.constInternalPointer())
    return false;

  switch (value.toInt()) {
    case Qt::CheckState::Checked:
      client.setPluginEnabled(pluginIndex(index.row()), true);
      emit dataChanged(index, index, { role });
      return true;
    case Qt::CheckState::Unchecked:
      client.setPluginEnabled(pluginIndex(index.row()), false);
      return true;
    default:
      return false;
  }
}

// Private methods

bool
PluginModel::isValidIndex(const QModelIndex& index) const noexcept
{
  return isValidColumn(index.column()) && index.row() >= 0 &&
         index.row() < pluginCount && index.constInternalPointer() == nullptr;
}
