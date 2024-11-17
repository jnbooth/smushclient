#include "aliasedit.h"
#include "ui_aliasedit.h"
#include "../../fieldconnector.h"

#define CONNECT(field) connectField(this, &alias, ui->field, alias.get##field(), &Alias::set##field);

// Public methods

AliasEdit::AliasEdit(Alias &alias, QWidget *parent)
    : QDialog(parent),
      ui(new Ui::AliasEdit),
      alias(alias),
      originalGroup(alias.getGroup())
{
  ui->setupUi(this);

  // Sender
  CONNECT(Group);
  CONNECT(Label);
  CONNECT(UserSendTo);
  CONNECT(Script);
  CONNECT(Variable);
  CONNECT(Enabled);
  CONNECT(OneShot);
  CONNECT(Temporary);
  CONNECT(OmitFromOutput);
  CONNECT(OmitFromLog);

  // Reaction
  CONNECT(Sequence);
  CONNECT(Pattern);
  CONNECT(IgnoreCase);
  CONNECT(KeepEvaluating);
  CONNECT(IsRegex);
  CONNECT(ExpandVariables);
  CONNECT(Repeats);

  // Alias
  CONNECT(EchoAlias);
  CONNECT(Menu);
  CONNECT(OmitFromCommandHistory);

  ui->Text->setPlainText(alias.getText());
}

AliasEdit::~AliasEdit()
{
  delete ui;
}

bool AliasEdit::groupChanged() const
{
  return originalGroup != ui->Group->text();
}

// Private slots

void AliasEdit::on_Label_textChanged(const QString &text)
{
  ui->Variable->setPlaceholderText(text);
}

void AliasEdit::on_UserSendTo_currentIndexChanged(int index)
{
  switch (index)
  {
  case (int)UserSendTarget::NotepadAppend:
  case (int)UserSendTarget::NotepadNew:
  case (int)UserSendTarget::NotepadReplace:
    ui->Variable->show();
    ui->Variable_label->setText(tr("Notepad:"));
    return;
  case (int)UserSendTarget::Variable:
    ui->Variable->show();
    ui->Variable_label->setText(tr("Variable:"));
    return;
  default:
    ui->Variable->hide();
    ui->Variable->clear();
    ui->Variable_label->clear();
  }
}

void AliasEdit::on_Text_textChanged()
{
  alias.setText(ui->Text->toPlainText());
}
