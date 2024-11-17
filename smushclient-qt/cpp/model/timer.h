#pragma once
#include "sender.h"

class Timekeeper;

class TimerModel : public AbstractSenderModel
{
  Q_OBJECT

public:
  TimerModel(SmushClient &client, Timekeeper *timekeeper, QObject *parent = nullptr);

protected:
  int addItem(SmushClient &client, QWidget *parent) override;
  int editItem(SmushClient &client, size_t index, QWidget *parent) override;
  QString exportXml(const SmushClient &client) const override;
  void importXml(SmushClient &client, const QString &xml) override;

private:
  Timekeeper *timekeeper;
};
