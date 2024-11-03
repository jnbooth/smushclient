#include "triggeredit.h"
#include "ui_triggeredit.h"
#include "../../fieldconnector.h"

#define CONNECT(field) connectField(this, &trigger, ui->field, trigger.get##field(), &Trigger::set##field);

TriggerEdit::TriggerEdit(Trigger &trigger, QWidget *parent)
    : QDialog(parent),
      ui(new Ui::TriggerEdit),
      trigger(trigger)
{
  ui->setupUi(this);

  // Sender
  CONNECT(Group);
  CONNECT(Label);
  CONNECT(SendToIndex);
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

  // Trigger
  CONNECT(ChangeForeground);
  CONNECT(ForegroundColor);
  CONNECT(ChangeBackground);
  CONNECT(BackgroundColor);
  CONNECT(MakeBold);
  CONNECT(MakeItalic);
  CONNECT(MakeUnderline);
  CONNECT(Sound);
  CONNECT(SoundIfInactive);
  CONNECT(LowercaseWildcard);
  CONNECT(MultiLine);
  CONNECT(LinesToMatch);

  ui->Text->setPlainText(trigger.getText());
}

TriggerEdit::~TriggerEdit()
{
  delete ui;
}

// Private methods

void TriggerEdit::on_Text_textChanged()
{
  trigger.setText(ui->Text->toPlainText());
}
