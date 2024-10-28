#include "finddialog.h"
#include "ui_finddialog.h"

FindDialog::FindDialog(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::FindDialog),
      filled(false),
      flags(),
      isRegex(false),
      pattern(),
      text()
{
  ui->setupUi(this);
  ui->Direction_Up->setChecked(true);
  hide();
}

FindDialog::~FindDialog()
{
  delete ui;
}

void FindDialog::find(QTextEdit *edit) const
{
  if (isRegex)
    edit->find(pattern, flags);
  else
    edit->find(text, flags);
}

void FindDialog::on_buttonBox_accepted()
{
  flags.setFlag(QTextDocument::FindFlag::FindBackward, ui->Direction_Up->isChecked());
  flags.setFlag(QTextDocument::FindFlag::FindCaseSensitively, ui->MatchCase->isChecked());
  isRegex = ui->RegularExpression->isChecked();
  text = ui->Find->text();
  filled = !text.isEmpty();
  if (isRegex && filled)
    pattern = QRegularExpression(text);
}

void FindDialog::on_buttonBox_rejected()
{
}
