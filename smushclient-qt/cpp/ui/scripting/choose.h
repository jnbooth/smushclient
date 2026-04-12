#pragma once
#include "abstractscriptdialog.h"

namespace Ui {
class ChooseDialog;
} // namespace Ui

class ChooseDialog : public AbstractScriptDialog
{
  Q_OBJECT

public:
  explicit ChooseDialog(const QString& title,
                        const QString& message,
                        QWidget* parent);
  ~ChooseDialog() override;

  void addItem(const QString& text,
               const QVariant& value,
               bool active) override;
  int pushValue(lua_State* L) const override;
  void sortItems() override;

private:
  Ui::ChooseDialog* ui;
};
