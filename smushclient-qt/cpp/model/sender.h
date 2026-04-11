#pragma once
#include "rust/cxx.h"
#include "smushclient_qt/src/ffi/regex.cxx.h"
#include <QtCore/QAbstractItemModel>
#include <QtCore/QItemSelection>

class SenderMap;
enum class SenderType;
class SmushClient;
class Timekeeper;

class AbstractSenderModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  AbstractSenderModel(SmushClient& client,
                      SenderType type,
                      QObject* parent = nullptr);
  bool addItem(QWidget* parent = nullptr);
  bool editItem(const QModelIndex& index, QWidget* parent = nullptr);
  virtual QString tryExportXml() const = 0;
  ParseResult importXml(const QString& xml);
  bool removeSelection(const QItemSelection& selection);
  int senderIndex(const QModelIndex& index) const;
  int columnCount(const QModelIndex& index = {}) const override;
  QVariant data(const QModelIndex& index,
                int role = Qt::DisplayRole) const override;
  bool hasChildren(const QModelIndex& index = {}) const override;
  QVariant headerData(int section,
                      Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;
  QModelIndex index(int row,
                    int column,
                    const QModelIndex& parent = {}) const override;
  QMap<int, QVariant> itemData(const QModelIndex& index) const override;
  QModelIndex parent(const QModelIndex& index) const override;
  bool removeRows(int row, int count, const QModelIndex& parent = {}) override;
  int rowCount(const QModelIndex& index = {}) const override;
  bool setData(const QModelIndex& index,
               const QVariant& value,
               int role = Qt::EditRole) override;

protected:
  static constexpr const int numColumns = 4;

  virtual bool add(QWidget* parent) = 0;
  virtual int edit(size_t index, QWidget* parent) = 0;
  virtual const std::array<QString, numColumns>& headers() const noexcept = 0;
  virtual ParseResult import(const QString& xml) = 0;
  virtual void prepareRemove(SenderMap& /*map*/,
                             const rust::String& /*group*/,
                             int /*row*/,
                             int /*count*/) {};

protected:
  SmushClient& client;

private:
  void refresh() const;
  bool removeRowsInternal(int row, int count, const QModelIndex& parent = {});

private:
  SenderMap* map;
  std::unique_ptr<bool> needsRefresh;
};
