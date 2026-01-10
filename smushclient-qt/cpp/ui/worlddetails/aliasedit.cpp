#include "aliasedit.h"
#include "../../fieldconnector.h"
#include "regexdialog.h"
#include "smushclient_qt/src/ffi/sender.cxxqt.h"
#include "smushclient_qt/src/ffi/util.cxx.h"
#include "ui_aliasedit.h"
#include <QtWidgets/QErrorMessage>

#define CONNECT(field)                                                         \
  connectField(this, &alias, ui->field, alias.get##field(), &Alias::set##field);

// Public methods

AliasEdit::AliasEdit(Alias& alias, QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::AliasEdit)
  , alias(alias)
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
  ui->Menu->setEnabled(!alias.getLabel().isEmpty());
}

AliasEdit::~AliasEdit()
{
  delete ui;
}

// Public slots

void
AliasEdit::accept()
{
  if (ui->IsRegex->isChecked()) {
    const QString pattern = ui->Pattern->text();
    const RegexError error = ffi::validateRegex(pattern);
    if (!error.message.isEmpty()) {
      RegexDialog dialog(error.message, error.offset, pattern, this);
      dialog.exec();
      return;
    }
  }
  QDialog::accept();
}

// Private slots

void
AliasEdit::on_Label_textChanged(const QString& text)
{
  ui->Menu->setEnabled(!text.isEmpty());
  ui->Variable->setPlaceholderText(text);
}

void
AliasEdit::on_UserSendTo_currentIndexChanged(int index)
{
  switch (index) {
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

void
AliasEdit::on_Text_textChanged()
{
  alias.setText(ui->Text->toPlainText());
}
