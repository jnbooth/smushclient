#pragma once
#include <QtCore/QAbstractItemModel>
#include <QtCore/QItemSelection>
#include <QtCore/QStringList>

class SenderMap;
enum class SenderType;
class SmushClient;
class Timekeeper;

class AbstractSenderModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  AbstractSenderModel(SmushClient &client, SenderType type, QObject *parent = nullptr);
  virtual ~AbstractSenderModel();
  bool addItem(QWidget *parent = nullptr);
  bool editItem(const QModelIndex &index, QWidget *parent = nullptr);
  QString exportXml() const;
  void importXml(const QString &xml);
  bool removeSelection(const QItemSelection &selection);
  int senderIndex(const QModelIndex &index) const;

  int columnCount(const QModelIndex &) const override
  {
    return numColumns;
  };
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  bool hasChildren(const QModelIndex &index = QModelIndex()) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  QModelIndex index(int row, int column, const QModelIndex &parent) const override;
  QMap<int, QVariant> itemData(const QModelIndex &index) const override;
  QModelIndex parent(const QModelIndex &index) const override;
  bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
  int rowCount(const QModelIndex &index = QModelIndex()) const override;
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

protected:
  virtual int addItem(SmushClient &client, QWidget *parent) = 0;
  virtual int editItem(SmushClient &client, size_t index, QWidget *parent) = 0;
  virtual QString exportXml(const SmushClient &client) const = 0;
  virtual void importXml(SmushClient &client, const QString &xml) = 0;
  void setHeaders(const QString &h1, const QString &h2, const QString &h3, const QString &h4);

private:
  static constexpr int numColumns = 4;

  SmushClient &client;
  std::array<QString, numColumns> headers;
  SenderMap *map;
  bool *needsRefresh;
  QString untitledGroupName;

private:
  static constexpr bool isValidColumn(int column) noexcept
  {
    return column >= 0 && column < numColumns;
  }

  void refresh() const;
  bool removeRowsInternal(int row, int count, const QModelIndex &parent = QModelIndex());
};
