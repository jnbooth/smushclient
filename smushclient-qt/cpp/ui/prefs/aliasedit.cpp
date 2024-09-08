#include "aliasedit.h"
#include "ui_aliasedit.h"
#include "../../fieldconnector.h"

#define CONNECT(field) connectField(this, alias, ui->field, alias->get##field(), &Alias::set##field);

AliasEdit::AliasEdit(Alias *alias, QWidget *parent)
    : QDialog(parent),
      ui(new Ui::AliasEdit),
      alias(alias)
{
  ui->setupUi(this);

  // Sender
  CONNECT(Group);
  CONNECT(Label);
  CONNECT(SendTo);
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

  ui->Text->setPlainText(alias->getText());
}

AliasEdit::~AliasEdit()
{
  delete ui;
}

void AliasEdit::on_Text_textChanged()
{
  alias->setText(ui->Text->toPlainText());
}
