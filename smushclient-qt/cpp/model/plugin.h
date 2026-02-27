#pragma once
#include <QtCore/QAbstractTableModel>

class PluginDetails;
class SmushClient;

class PluginModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  explicit PluginModel(SmushClient& client, QObject* parent = nullptr);
  bool addPlugin(const QString& filePath);
  PluginDetails pluginDetails(const QModelIndex& index) const;
  bool reinstall(const QModelIndex& index);

  int columnCount(const QModelIndex& index = QModelIndex()) const override
  {
    return index.isValid() ? 0 : numColumns;
  }
  QVariant data(const QModelIndex& index,
                int role = Qt::DisplayRole) const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  QVariant headerData(int section,
                      Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;
  QMap<int, QVariant> itemData(const QModelIndex& index) const override;
  bool removeRows(int row,
                  int count,
                  const QModelIndex& parent = QModelIndex()) override;
  int rowCount(const QModelIndex& index = QModelIndex()) const override
  {
    return index.isValid() ? 0 : pluginCount - 1;
  }
  bool setData(const QModelIndex& index,
               const QVariant& value,
               int role = Qt::EditRole) override;

signals:
  void clientError(const QString& error);
  void pluginOrderChanged();
  void pluginScriptChanged(size_t pluginIndex);

protected:
  static constexpr int numColumns = 6;

private:
  size_t pluginIndex(int row) const noexcept
  {
    return row >= worldIndex ? row + 1 : row;
  }
  int pluginIndexToRow(int index) const noexcept
  {
    return index > worldIndex ? index - 1 : index;
  }

  SmushClient& client;
  int pluginCount;
  int worldIndex;
};
