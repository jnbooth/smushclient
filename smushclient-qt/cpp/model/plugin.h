#pragma once
#include <QtCore/QAbstractItemModel>

class PluginDetails;
class SmushClient;

class PluginModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  explicit PluginModel(SmushClient& client, QObject* parent = nullptr);
  bool addPlugin(const QString& filePath);
  PluginDetails pluginDetails(const QModelIndex& index) const;
  QString pluginId(const QModelIndex& index) const;
  size_t pluginIndex(int row) const noexcept
  {
    return row >= worldIndex ? row + 1 : row;
  }
  bool reinstall(const QModelIndex& index);

  int columnCount(const QModelIndex& /*index*/) const override
  {
    return numColumns;
  }
  QVariant data(const QModelIndex& index, int role) const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  bool hasChildren(const QModelIndex& index) const override;
  QVariant headerData(int section,
                      Qt::Orientation orientation,
                      int role) const override;
  QModelIndex index(int row,
                    int column,
                    const QModelIndex& parent) const override;
  QMap<int, QVariant> itemData(const QModelIndex& index) const override;
  QModelIndex parent(const QModelIndex& index) const override;
  bool removeRows(int row, int count, const QModelIndex& parent) override;
  int rowCount(const QModelIndex& index) const override;
  bool setData(const QModelIndex& index,
               const QVariant& value,
               int role) override;

signals:
  void clientError(const QString& error);
  void pluginOrderChanged();
  void pluginScriptChanged(size_t pluginIndex);

protected:
  static constexpr int numColumns = 6;

private:
  static constexpr bool isValidColumn(int column) noexcept
  {
    return column >= 0 && column < numColumns;
  }

  bool isValidIndex(const QModelIndex& index) const noexcept
  {
    return isValidColumn(index.column()) && index.row() >= 0 &&
           index.row() < pluginCount && index.internalId() == 0;
  }

  int pluginIndexToRow(int index) const noexcept
  {
    return index > worldIndex ? index - 1 : index;
  }

  SmushClient& client;
  int pluginCount;
  int worldIndex;
};
