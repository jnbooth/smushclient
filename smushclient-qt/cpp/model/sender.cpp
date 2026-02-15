#include "sender.h"
#include "../client.h"
#include "smushclient_qt/src/ffi/sender_map.cxxqt.h"
#include <QtCore/QAbstractProxyModel>

struct SelectionRegion
{
  int top = -1;
  int height = -1;
  const void* parentPtr = nullptr;

  SelectionRegion() = default;

  SelectionRegion(const QItemSelectionRange& range, const QModelIndex& parent)
    : top(range.top())
    , height(range.height())
    , parentPtr(parent.constInternalPointer())
  {
  }

  bool operator==(const SelectionRegion&) const = default;
};

// Private utils

namespace {
const rust::String*
getGroup(const QModelIndex& modelIndex)
{
  return static_cast<const rust::String*>(modelIndex.constInternalPointer());
}
} // namespace

// Public methods

AbstractSenderModel::AbstractSenderModel(SmushClient& client,
                                         SenderType type,
                                         QObject* parent)
  : QAbstractItemModel(parent)
  , client(client)
  , map(new SenderMap(type))
  , needsRefresh(std::make_unique<bool>(true))
{
  map->setParent(this);
}

bool
AbstractSenderModel::addItem(QWidget* parent)
{
  if (!add(parent)) {
    return false;
  }

  beginResetModel();
  *needsRefresh = true;
  endResetModel();
  return true;
}

bool
AbstractSenderModel::editItem(const QModelIndex& modelIndex, QWidget* parent)
{
  refresh();
  const rust::String* group = getGroup(modelIndex);
  if (group == nullptr) {
    return false;
  }

  const int row = modelIndex.row();
  if (row < 0) {
    return false;
  }

  const int index = map->senderIndex(*group, row);
  if (index < 0) {
    return false;
  }

  const int newIndex = edit(index, parent);

  if (newIndex == index) {
    emit dataChanged(modelIndex.siblingAtColumn(0),
                     modelIndex.siblingAtColumn(numColumns - 1),
                     { Qt::DisplayRole });
    return true;
  }

  if (newIndex == static_cast<int>(ReplaceSenderResult::GroupChanged)) {
    beginResetModel();
    *needsRefresh = true;
    endResetModel();
    return true;
  }

  if (newIndex < 0) {
    return false;
  }

  *needsRefresh = true;
  refresh();

  const int newRow = map->positionInGroup(*group, newIndex);
  if (newRow == modelIndex.row()) {
    emit dataChanged(modelIndex.siblingAtColumn(0),
                     modelIndex.siblingAtColumn(numColumns - 1),
                     { Qt::DisplayRole, Qt::EditRole });
    return true;
  }

  emit layoutAboutToBeChanged(
    {}, QAbstractItemModel::LayoutChangeHint::VerticalSortHint);

  const QModelIndex parentIndex = modelIndex.parent();

  beginMoveRows(parentIndex, row, row, parentIndex, newRow);
  endMoveRows();

  emit dataChanged(modelIndex.sibling(newRow, 0),
                   modelIndex.sibling(newRow, numColumns - 1),
                   { Qt::DisplayRole, Qt::EditRole });

  emit layoutChanged({},
                     QAbstractItemModel::LayoutChangeHint::VerticalSortHint);
  return true;
}

RegexParse
AbstractSenderModel::tryImportXml(const QString& xml)
{
  beginResetModel();
  const RegexParse result = tryImport(xml);
  *needsRefresh = true;
  endResetModel();
  return result;
}

bool
AbstractSenderModel::removeSelection(const QItemSelection& selection)
{
  if (selection.isEmpty()) {
    return false;
  }

  refresh();
  bool succeeded = false;
  SelectionRegion lastRegion;
  emit layoutAboutToBeChanged({});
  for (const QItemSelectionRange& range : selection) {
    const QModelIndex parent = range.parent();

    const SelectionRegion currentRegion(range, parent);
    if (currentRegion == lastRegion) {
      continue;
    }
    lastRegion = currentRegion;

    succeeded = removeRowsInternal(lastRegion.top, lastRegion.height, parent) ||
                succeeded;
  }
  emit layoutChanged();
  return succeeded;
}

int
AbstractSenderModel::senderIndex(const QModelIndex& index) const
{
  refresh();
  const rust::String* group = getGroup(index);
  if (group == nullptr) {
    return -1;
  }

  const int row = index.row();
  if (row < 0) {
    return -1;
  }

  return map->senderIndex(*group, row);
}

// Public overrides

QVariant
AbstractSenderModel::data(const QModelIndex& index, int role) const
{
  refresh();
  if (role != Qt::DisplayRole && role != Qt::EditRole) {
    return QVariant();
  }

  const rust::String* group = getGroup(index);
  if (group != nullptr) {
    return map->cellText(client, *group, index.row(), index.column());
  }

  if (index.column() != 0) {
    return QVariant();
  }

  const rust::String* parentGroup = map->groupName(index.row());
  if (parentGroup == nullptr) {
    return QString();
  }

  const size_t len = parentGroup->length();
  if (len != 0) {
    return QString::fromUtf8(parentGroup->data(), static_cast<qsizetype>(len));
  }

  static const QString untitledGroupName = tr("(ungrouped)");

  return untitledGroupName;
}

bool
AbstractSenderModel::hasChildren(const QModelIndex& index) const
{
  return index.constInternalPointer() == nullptr;
}

QVariant
AbstractSenderModel::headerData(int section,
                                Qt::Orientation orientation,
                                int role) const
{
  if (orientation != Qt::Orientation::Horizontal || !isValidColumn(section)) {
    return QVariant();
  }

  if (role == Qt::DisplayRole) {
    return headers().at(section);
  }
  return QVariant();
}

QModelIndex
AbstractSenderModel::index(int row, int column, const QModelIndex& parent) const
{
  refresh();

  if (row < 0 || !isValidColumn(column)) {
    return QModelIndex();
  }

  if (!parent.isValid()) {
    return (map->groupName(row) != nullptr) ? createIndex(row, column)
                                            : QModelIndex();
  }

  if ((parent.column() != 0) || (parent.constInternalPointer() != nullptr)) {
    return QModelIndex();
  }

  const int parentRow = parent.row();
  if (parentRow < 0) {
    return QModelIndex();
  }

  const rust::String* groupName = map->groupName(parentRow);
  if (groupName == nullptr) {
    return QModelIndex();
  }

  return createIndex(row, column, groupName);
}

QMap<int, QVariant>
AbstractSenderModel::itemData(const QModelIndex& index) const
{
  refresh();
  QMap<int, QVariant> dataMap;
  dataMap.insert(Qt::DisplayRole, data(index, Qt::DisplayRole));
  return dataMap;
}

QModelIndex
AbstractSenderModel::parent(const QModelIndex& index) const
{
  refresh();
  const rust::String* group = getGroup(index);
  if (group == nullptr) {
    return QModelIndex();
  }

  const int row = map->groupIndex(*group);
  if (row == -1) {
    return QModelIndex();
  }
  return createIndex(row, 0);
}

bool
AbstractSenderModel::removeRows(int row, int count, const QModelIndex& parent)
{
  refresh();
  emit layoutAboutToBeChanged({});
  const bool succeeded = removeRowsInternal(row, count, parent);
  emit layoutChanged({});
  return succeeded;
}

int
AbstractSenderModel::rowCount(const QModelIndex& index) const
{
  refresh();
  if (!index.isValid()) {
    return static_cast<int>(map->len());
  }

  if (index.constInternalPointer() != nullptr) {
    return 0;
  }

  return static_cast<int>(map->groupLen(index.row()));
}

bool
AbstractSenderModel::setData(const QModelIndex& index,
                             const QVariant& value,
                             int role)
{
  if (role != Qt::EditRole) {
    return false;
  }

  refresh();

  const rust::String* group = getGroup(index);
  if (group == nullptr) {
    return false;
  }

  const int row = index.row();
  if (row < 0) {
    return false;
  }

  const int newRow = map->setCell(client, *group, row, index.column(), value);
  if (newRow < 0) {
    return false;
  }

  static const QList<int> changedRoles{ Qt::DisplayRole, Qt::EditRole };

  if (newRow == row) {
    emit dataChanged(index, index, changedRoles);
    return true;
  }

  emit layoutAboutToBeChanged(
    {}, QAbstractItemModel::LayoutChangeHint::VerticalSortHint);

  const QModelIndex parentIndex = index.parent();

  beginMoveRows(parentIndex, row, row, parentIndex, newRow);
  endMoveRows();

  const QModelIndex newIndex = index.siblingAtRow(newRow);

  emit dataChanged(newIndex, newIndex, changedRoles);

  emit layoutChanged({},
                     QAbstractItemModel::LayoutChangeHint::VerticalSortHint);
  return true;
}

// Private methods

void
AbstractSenderModel::refresh() const
{
  if (!*needsRefresh) {
    return;
  }
  *needsRefresh = false;
  map->recalculate(client);
}

bool
AbstractSenderModel::removeRowsInternal(int row,
                                        int count,
                                        const QModelIndex& parent)
{
  if (row < 0 || count <= 0) {
    return false;
  }

  const rust::String* group = map->groupName(parent.row());
  if (group == nullptr) {
    return false;
  }

  prepareRemove(*map, *group, row, count);
  beginRemoveRows(parent, row, row + count - 1);
  const bool succeeded = map->remove(client, *group, row, count);
  *needsRefresh = true;
  endRemoveRows();
  return succeeded;
}
