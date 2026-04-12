#include "choose.h"
#include "../scripting/qlua.h"
#include "ui_choose.h"

// Public methods

ChooseDialog::ChooseDialog(const QString& title,
                           const QString& message,
                           QWidget* parent)
  : AbstractScriptDialog(parent)
  , ui(new Ui::ChooseDialog)
{
  ui->setupUi(this);
  setWindowTitle(title);
  ui->label->setText(message);
}

ChooseDialog::~ChooseDialog()
{
  delete ui;
}

// Public overrides

void
ChooseDialog::addItem(const QString& text, const QVariant& value, bool active)
{
  ui->items->addItem(text, value);
  if (active) {
    ui->items->setCurrentIndex(ui->items->count() - 1);
  }
}

int
ChooseDialog::pushValue(lua_State* L) const
{
  qlua::pushQVariant(L, ui->items->currentData());
  return 1;
}

void
ChooseDialog::sortItems()
{
  ui->items->model()->sort(0);
}
