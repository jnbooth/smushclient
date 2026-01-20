#pragma once
#include "sender.h"

class TriggerModel : public AbstractSenderModel
{
  Q_OBJECT

public:
  explicit TriggerModel(SmushClient& client, QObject* parent = nullptr);

  QString exportXml() const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;

protected:
  bool add(QWidget* parent) override;
  int edit(size_t index, QWidget* parent) override;
  const std::array<QString, AbstractSenderModel::numColumns>& headers()
    const noexcept override;
  RegexParse import(const QString& xml) override;
};
