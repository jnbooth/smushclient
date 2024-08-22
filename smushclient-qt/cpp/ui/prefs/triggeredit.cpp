#include "triggeredit.h"
#include "ui_triggeredit.h"

TriggerEdit::TriggerEdit(QWidget *parent)
    : QDialog(parent), ui(new Ui::TriggerEdit)
{
  ui->setupUi(this);
}

TriggerEdit::~TriggerEdit()
{
  delete ui;
}

void TriggerEdit::on_Text_textChanged()
{
}
