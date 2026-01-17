#pragma once
#include "sender.h"

class Timekeeper;

class TimerModel : public AbstractSenderModel
{
  Q_OBJECT

public:
  TimerModel(SmushClient& client,
             Timekeeper* timekeeper,
             QObject* parent = nullptr);

  QString exportXml() const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;

protected:
  bool add(QWidget* parent) override;
  int edit(size_t index, QWidget* parent) override;
  const std::array<QString, AbstractSenderModel::numColumns>& headers()
    const noexcept override;
  RegexParse import(const QString& xml) override;
  void prepareRemove(SenderMap* map,
                     const rust::String& group,
                     int row,
                     int count) override;

private:
  Timekeeper* timekeeper;
};
