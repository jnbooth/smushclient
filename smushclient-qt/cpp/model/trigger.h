#pragma once
#include "sender.h"

class TriggerModel : public AbstractSenderModel
{
  Q_OBJECT

public:
  TriggerModel(SmushClient &client, QObject *parent = nullptr);

  Qt::ItemFlags flags(const QModelIndex &index) const override;

protected:
  int addItem(SmushClient &client, QWidget *parent) override;
  int editItem(SmushClient &client, size_t index, QWidget *parent) override;
  QString exportXml(const SmushClient &client) const override;
  void importXml(SmushClient &client, const QString &xml) override;
};
