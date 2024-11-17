#include "sender.h"
#include <QtCore/QAbstractProxyModel>
#include "../ui/worlddetails/aliasedit.h"
#include "../ui/worlddetails/timeredit.h"
#include "../ui/worlddetails/triggeredit.h"
#include "cxx-qt-gen/ffi.cxxqt.h"

using std::array;
using std::vector;

struct SelectionRegion
{
  int top;
  int height;
  const void *parentPtr;

  bool operator==(const SelectionRegion &) const = default;

  SelectionRegion()
      : top(-1),
        height(-1),
        parentPtr(nullptr) {}

  SelectionRegion(const QItemSelectionRange &range, const QModelIndex &parent)
      : top(range.top()),
        height(range.height()),
        parentPtr(parent.constInternalPointer()) {}
};

// Public methods

AbstractSenderModel::AbstractSenderModel(SmushClient &client, SenderType type, QObject *parent)
    : QAbstractItemModel(parent),
      client(client),
      needsRefresh(new bool(true))
{
  untitledGroupName = tr("(ungrouped)");
  map = new SenderMap(type);
  map->setParent(this);
}

AbstractSenderModel::~AbstractSenderModel()
{
  delete needsRefresh;
}

bool AbstractSenderModel::addItem(QWidget *parent)
{
  if (addItem(client, parent) < 0)
    return false;

  beginResetModel();
  *needsRefresh = true;
  endResetModel();
  return true;
}

bool AbstractSenderModel::editItem(const QModelIndex &modelIndex, QWidget *parent)
{
  qDebug() << "edit" << modelIndex;
  refresh();
  const rust::String *group = static_cast<const rust::String *>(modelIndex.constInternalPointer());
  if (!group)
    return false;

  const int row = modelIndex.row();
  if (row < 0)
    return false;

  const int index = map->senderIndex(*group, row);
  if (index < 0)
    return false;

  const int newIndex = editItem(client, index, parent);

  if (newIndex == index)
  {
    emit dataChanged(
        modelIndex.siblingAtColumn(0),
        modelIndex.siblingAtColumn(numColumns - 1),
        {Qt::DisplayRole});
    return true;
  }

  if (newIndex == -1)
    return false;

  if (newIndex < 0)
  {
    beginResetModel();
    *needsRefresh = true;
    endResetModel();
    return true;
  }

  *needsRefresh = true;
  refresh();

  const int newRow = map->positionInGroup(*group, newIndex);
  if (newRow == modelIndex.row())
  {
    emit dataChanged(
        modelIndex.siblingAtColumn(0),
        modelIndex.siblingAtColumn(numColumns - 1),
        {Qt::DisplayRole});
    return true;
  }

  emit layoutAboutToBeChanged({}, QAbstractItemModel::LayoutChangeHint::VerticalSortHint);

  const QModelIndex parentIndex = modelIndex.parent();

  beginMoveRows(parentIndex, row, row, parentIndex, newRow);
  endMoveRows();

  emit dataChanged(
      modelIndex.sibling(newRow, 0),
      modelIndex.sibling(newRow, numColumns - 1),
      {Qt::DisplayRole});

  emit layoutChanged({}, QAbstractItemModel::LayoutChangeHint::VerticalSortHint);
  return true;
}

QString AbstractSenderModel::exportXml() const
{
  return exportXml(client);
}

void AbstractSenderModel::importXml(const QString &xml)
{
  beginResetModel();
  importXml(client, xml);
  *needsRefresh = true;
  endResetModel();
}

bool AbstractSenderModel::removeSelection(const QItemSelection &selection)
{
  if (selection.isEmpty())
    return false;

  refresh();
  bool succeeded = false;
  SelectionRegion lastRegion;
  emit layoutAboutToBeChanged({});
  for (auto it = selection.crbegin(), end = selection.crend(); it < end; ++it)
  {
    const QItemSelectionRange &range = *it;
    const QModelIndex parent = range.parent();

    const SelectionRegion currentRegion(range, parent);
    if (currentRegion == lastRegion)
      continue;
    lastRegion = currentRegion;

    succeeded = removeRowsInternal(lastRegion.top, lastRegion.height, parent) || succeeded;
  }
  emit layoutChanged();
  return succeeded;
}

int AbstractSenderModel::senderIndex(const QModelIndex &index) const
{
  refresh();
  const rust::String *group = static_cast<const rust::String *>(index.constInternalPointer());
  if (!group)
    return -1;

  const int row = index.row();
  if (row < 0)
    return -1;

  return map->senderIndex(*group, row);
}

// Public overrides

QVariant AbstractSenderModel::data(const QModelIndex &index, int role) const
{
  refresh();
  if (role != Qt::DisplayRole)
    return QVariant();

  const rust::String *group = static_cast<const rust::String *>(index.constInternalPointer());
  if (group)
    return map->cellText(client, *group, index.row(), index.column());

  if (index.column())
    return QVariant();

  const rust::String *parentGroup = map->groupName(index.row());
  if (!parentGroup)
    return QString();

  size_t len = parentGroup->length();
  if (len)
    return QString::fromUtf8(parentGroup->data(), len);

  return untitledGroupName;
}

Qt::ItemFlags AbstractSenderModel::flags(const QModelIndex &index) const
{
  return index.constInternalPointer()
             ? Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemNeverHasChildren
             : Qt::ItemFlag::ItemIsEnabled;
}

bool AbstractSenderModel::hasChildren(const QModelIndex &index) const
{
  return !index.constInternalPointer();
}

QVariant AbstractSenderModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation != Qt::Orientation::Horizontal || !isValidColumn(section))
    return QVariant();

  switch (role)
  {
  case Qt::DisplayRole:
    return headers[section];
  default:
    return QVariant();
  }
}

QModelIndex AbstractSenderModel::index(int row, int column, const QModelIndex &parent) const
{
  refresh();

  if (row < 0 || !isValidColumn(column))
    return QModelIndex();

  if (!parent.isValid())
    return map->groupName(row) ? createIndex(row, column) : QModelIndex();

  if (parent.column() || parent.constInternalPointer())
    return QModelIndex();

  const int parentRow = parent.row();
  if (parentRow < 0)
    return QModelIndex();

  const rust::String *groupName = map->groupName(parentRow);
  if (!groupName)
    return QModelIndex();

  return createIndex(row, column, groupName);
}

QMap<int, QVariant> AbstractSenderModel::itemData(const QModelIndex &index) const
{
  refresh();
  QMap<int, QVariant> map;
  map.insert(Qt::DisplayRole, data(index, Qt::DisplayRole));
  return map;
}

QModelIndex AbstractSenderModel::parent(const QModelIndex &index) const
{
  refresh();
  const rust::String *group = static_cast<const rust::String *>(index.constInternalPointer());
  if (!group)
    return QModelIndex();

  const int row = map->groupIndex(*group);
  if (row == -1)
    return QModelIndex();
  return createIndex(row, 0);
}

bool AbstractSenderModel::removeRows(int row, int count, const QModelIndex &parent)
{
  refresh();
  emit layoutAboutToBeChanged({});
  const bool succeeded = removeRowsInternal(row, count, parent);
  emit layoutChanged({});
  return succeeded;
}

int AbstractSenderModel::rowCount(const QModelIndex &index) const
{
  refresh();
  if (!index.isValid())
    return map->len();

  if (index.constInternalPointer())
    return 0;

  return map->groupLen(index.row());
}

// Protected methods

void AbstractSenderModel::setHeaders(const QString &h1, const QString &h2, const QString &h3, const QString &h4)
{
  headers = {h1, h2, h3, h4};
}

// Private methods

void AbstractSenderModel::refresh() const
{
  if (!*needsRefresh)
    return;
  *needsRefresh = false;
  map->recalculate(client);
}

bool AbstractSenderModel::removeRowsInternal(int row, int count, const QModelIndex &parent)
{
  if (row < 0 || count <= 0)
    return false;

  const rust::String *group = map->groupName(parent.row());
  if (!group)
    return false;

  beginRemoveRows(parent, row, row + count - 1);
  const bool succeeded = map->remove(client, *group, row, count);
  *needsRefresh = true;
  endRemoveRows();
  return succeeded;
}