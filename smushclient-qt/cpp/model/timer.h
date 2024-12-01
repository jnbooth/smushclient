#pragma once
#include "sender.h"

class Timekeeper;

class TimerModel : public AbstractSenderModel
{
  Q_OBJECT

public:
  TimerModel(SmushClient &client, Timekeeper *timekeeper, QObject *parent = nullptr);

  QString exportXml() const override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;

protected:
  int add(QWidget *parent) override;
  int edit(size_t index, QWidget *parent) override;
  void import(const QString &xml) override;

private:
  Timekeeper *timekeeper;
};
