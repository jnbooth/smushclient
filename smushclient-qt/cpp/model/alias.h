#pragma once
#include "sender.h"

class AliasModel : public AbstractSenderModel
{
  Q_OBJECT

public:
  explicit AliasModel(SmushClient& client, QObject* parent = nullptr);

protected:
  bool add(QWidget* parent) override;
  int edit(size_t index, QWidget* parent) override;
  ParseResult import(const QString& xml) override;
};
