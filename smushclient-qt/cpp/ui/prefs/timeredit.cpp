#include "timeredit.h"
#include "ui_timeredit.h"
#include "../../enumbuttongroup.h"
#include "../../fieldconnector.h"

#define CONNECT(field) connectField(this, &timer, ui->field, timer.get##field(), &Timer::set##field);

TimerEdit::TimerEdit(Timer &timer, QWidget *parent)
    : QDialog(parent),
      ui(new Ui::TimerEdit),
      timer(timer)
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

  // Timer
  CONNECT(AtTime);
  CONNECT(EveryHour);
  CONNECT(EveryMinute);
  CONNECT(EverySecond);
  CONNECT(ActiveClosed);

  ui->Text->setPlainText(timer.getText());
  EnumButtonGroup(this, timer.getOccurrence(), &TimerEdit::on_OccurrenceChanged)
      .addButton(ui->Occurrence_Interval, Occurrence::Interval)
      .addButton(ui->Occurrence_Time, Occurrence::Time);
}

TimerEdit::~TimerEdit()
{
  delete ui;
}

// Private methods

void TimerEdit::on_Text_textChanged()
{
  timer.setText(ui->Text->toPlainText());
}

void TimerEdit::on_OccurrenceChanged(Occurrence value)
{
  timer.setOccurrence(value);
}
