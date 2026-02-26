#include "plugin.h"
#include "../client.h"
#include "smushclient_qt/src/ffi/plugin_details.cxxqt.h"

using std::array;

// Private utils
namespace {
inline bool
isLeaf(const QModelIndex& index) noexcept
{
  return index.constInternalPointer() != nullptr;
}
} // namespace

// Public methods

PluginModel::PluginModel(SmushClient& client, QObject* parent)
  : QAbstractItemModel(parent)
  , client(client)
  , pluginCount(static_cast<int>(client.pluginsLen()) - 1)
  , worldIndex(static_cast<int>(client.worldPluginIndex()))
{
}

bool
PluginModel::addPlugin(const QString& filePath)
{
  int pluginIndex;
  try {
    pluginIndex = static_cast<int>(client.tryAddPlugin(filePath));
  } catch (const rust::Error& e) {
    const QString error = QString::fromUtf8(e.what());
    emit clientError(error);
    return false;
  }
  emit layoutAboutToBeChanged();
  if (pluginIndex <= worldIndex) {
    ++worldIndex;
  }
  const int row = pluginIndexToRow(pluginIndex);
  beginInsertRows(createIndex(0, 0), row, row);
  ++pluginCount;
  endInsertRows();
  if (pluginIndex + 1 != pluginCount) {
    emit pluginOrderChanged();
  }
  emit layoutChanged();
  return true;
}

PluginDetails
PluginModel::pluginDetails(const QModelIndex& index) const
{
  if (!isValidIndex(index)) {
    return PluginDetails();
  }
  return PluginDetails(client, pluginIndex(index.row()));
}

bool
PluginModel::reinstall(const QModelIndex& index)
{
  if (!isValidIndex(index)) {
    return false;
  }
  const int row = index.row();
  const size_t oldIndex = pluginIndex(row);
  size_t newIndex;
  try {
    newIndex = client.tryReinstallPlugin(oldIndex);
  } catch (const rust::Error& e) {
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
  if (oldIndex > worldPluginIndex && newIndex <= worldPluginIndex) {
    ++worldIndex;
  } else if (oldIndex < worldPluginIndex && newIndex >= worldPluginIndex) {
    --worldIndex;
  }
  const int newRow = pluginIndexToRow(static_cast<int>(newIndex));
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
  if (!isValidIndex(index)) {
    return QVariant();
  }

  switch (role) {
    case Qt::DisplayRole:
      return client.pluginModelText(pluginIndex(index.row()), index.column());
    case Qt::CheckStateRole:
      if (index.column() != 4) {
        return QVariant();
      }
      return client.pluginEnabled(pluginIndex(index.row()))
               ? Qt::CheckState::Checked
               : Qt::CheckState::Unchecked;
    case Qt::InitialSortOrderRole:
      return static_cast<int>(pluginIndex(index.row()));
    default:
      return QVariant();
  }
}

Qt::ItemFlags
PluginModel::flags(const QModelIndex& index) const
{
  if (!isValidIndex(index)) {
    return Qt::ItemFlag::NoItemFlags;
  }

  const Qt::ItemFlags flags = Qt::ItemFlag::ItemIsSelectable |
                              Qt::ItemFlag::ItemIsEnabled |
                              Qt::ItemFlag::ItemNeverHasChildren;

  if (index.column() != 4) {
    return flags;
  }

  return flags | Qt::ItemFlag::ItemIsUserCheckable;
}

bool
PluginModel::hasChildren(const QModelIndex& /*index*/) const
{
  return false;
}

QVariant
PluginModel::headerData(int section,
                        Qt::Orientation orientation,
                        int role) const
{
  if (orientation != Qt::Orientation::Horizontal || !isValidColumn(section)) {
    return QVariant();
  }

  const static array<QString, numColumns> headers{
    tr("Name"), tr("Purpose"), tr("Author"),
    tr("Path"), tr("Enabled"), tr("Version")
  };

  if (role == Qt::DisplayRole) {
    return headers.at(section);
  }

  return QVariant();
}

QModelIndex
PluginModel::index(int row, int column, const QModelIndex& parent) const
{
  if (!isValidColumn(column) || (row < 0) || (row >= rowCount(parent))) {
    return QModelIndex();
  }

  return createIndex(row, column);
}

QMap<int, QVariant>
PluginModel::itemData(const QModelIndex& index) const
{
  if (!isValidIndex(index)) {
    return QMap<int, QVariant>();
  }

  QMap<int, QVariant> map;
  map.insert(Qt::DisplayRole, data(index, Qt::DisplayRole));
  map.insert(Qt::CheckStateRole, data(index, Qt::CheckStateRole));
  map.insert(Qt::InitialSortOrderRole, data(index, Qt::InitialSortOrderRole));
  return map;
}

QModelIndex
PluginModel::parent(const QModelIndex& /*index*/) const
{
  return QModelIndex();
}

bool
PluginModel::removeRows(int row, int count, const QModelIndex& parent)
{
  if (row < 0 || count <= 0 || row + count > pluginCount || isLeaf(parent)) {
    return false;
  }

  emit layoutAboutToBeChanged();
  beginRemoveRows(parent, row, row + count - 1);
  bool succeeded = true;
  int i = 0;
  for (; i < count && succeeded; ++i) {
    if (row >= worldIndex) {
      succeeded = client.removePlugin(row + 1);
    } else {
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
  return isLeaf(index) ? 0 : pluginCount;
}

bool
PluginModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (role != Qt::CheckStateRole || index.column() != 4 || index.row() < 0 ||
      index.row() + 1 >= pluginCount || isLeaf(index)) {
    return false;
  }

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
