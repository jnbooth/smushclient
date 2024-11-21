#include "timeredit.h"
#include "ui_timeredit.h"
#include "../../enumbuttongroup.h"
#include "../../fieldconnector.h"
#include "cxx-qt-gen/ffi.cxxqt.h"

#define CONNECT(field) connectField(this, &timer, ui->field, timer.get##field(), &Timer::set##field);

TimerEdit::TimerEdit(Timer &timer, QWidget *parent)
    : QDialog(parent),
      ui(new Ui::TimerEdit),
      originalGroup(timer.getGroup()),
      timer(timer)
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

bool TimerEdit::groupChanged() const
{
  return originalGroup != ui->Group->text();
}

// Private slots

void TimerEdit::on_OccurrenceChanged(Occurrence value)
{
  timer.setOccurrence(value);
}

void TimerEdit::on_Label_textChanged(const QString &text)
{
  ui->Variable->setPlaceholderText(text);
}

void TimerEdit::on_UserSendTo_currentIndexChanged(int index)
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
    ui->Variable_label->clear();
  }
}

void TimerEdit::on_Text_textChanged()
{
  timer.setText(ui->Text->toPlainText());
}
