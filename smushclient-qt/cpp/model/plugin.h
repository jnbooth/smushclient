#pragma once
#include <QtCore/QAbstractItemModel>

class PluginDetails;
class SmushClient;

class PluginModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  explicit PluginModel(SmushClient &client, QObject *parent = nullptr);
  bool addPlugin(const QString &filePath);
  PluginDetails pluginDetails(const QModelIndex &index) const;
  QString pluginId(const QModelIndex &index) const;
  bool reinstall(const QModelIndex &index);
  constexpr size_t pluginIndex(int row) const
  {
    return row >= worldIndex ? row + 1 : row;
  }

  int columnCount(const QModelIndex &) const override
  {
    return numColumns;
  }
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  bool hasChildren(const QModelIndex &index = QModelIndex()) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  QModelIndex index(int row, int column, const QModelIndex &parent) const override;
  QMap<int, QVariant> itemData(const QModelIndex &index) const override;
  QModelIndex parent(const QModelIndex &index) const override;
  bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
  int rowCount(const QModelIndex &index = QModelIndex()) const override;
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

signals:
  void clientError(const QString &error);
  void pluginOrderChanged();
  void pluginScriptChanged(size_t pluginIndex);

private:
  static constexpr int numColumns = 6;

  SmushClient &client;
  std::array<QString, numColumns> headers;
  int pluginCount;
  int worldIndex;

private:
  static constexpr bool isValidColumn(int column) noexcept
  {
    return column >= 0 && column < numColumns;
  }

  bool isValidIndex(const QModelIndex &index) const noexcept;

  constexpr int pluginIndexToRow(int index) const
  {
    return index > worldIndex ? index - 1 : index;
  }
};
