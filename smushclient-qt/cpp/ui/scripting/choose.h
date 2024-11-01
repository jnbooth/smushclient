#pragma once
#include "abstractscriptdialog.h"

namespace Ui
{
  class Choose;
}

class Choose : public AbstractScriptDialog
{
  Q_OBJECT

public:
  explicit Choose(const QString &title, const QString &message, QWidget *parent = nullptr);
  ~Choose();

  void addItem(const QString &text, const QVariant &value, bool active = false) override;
  void sortItems() override;
  QVariant value() const override;

private:
  Ui::Choose *ui;
};
