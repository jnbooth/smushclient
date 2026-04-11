#include "aliasedit.h"
#include "../../fieldconnector.h"
#include "../../settings.h"
#include "../dialog/regexdialog.h"
#include "smushclient_qt/src/ffi/regex.cxx.h"
#include "smushclient_qt/src/ffi/sender.cxxqt.h"
#include "ui_aliasedit.h"
#include <QtWidgets/QErrorMessage>

#define CONNECT(field)                                                         \
  FieldConnector::connect(                                                     \
    this, &alias, ui->field, alias.get##field(), &Alias::set##field);

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

  ui->Text->setFont(Settings().getScriptFont());
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
  if (ui->IsRegex->isChecked() &&
      !RegexDialog::handle(ffi::regex::validate(ui->Pattern->text()))) {
    return;
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
AliasEdit::on_SendTo_currentIndexChanged(int index)
{
  switch (static_cast<SendTarget>(index)) {
    case SendTarget::NotepadAppend:
    case SendTarget::NotepadNew:
    case SendTarget::NotepadReplace:
      ui->Variable->show();
      ui->Variable_label->setText(tr("Notepad:"));
      return;
    case SendTarget::Variable:
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
