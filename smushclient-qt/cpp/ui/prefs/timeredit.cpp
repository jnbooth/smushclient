#include "timeredit.h"
#include "ui_timeredit.h"
#include "../../fieldconnector.h"
#include <QtWidgets/QButtonGroup>

#define CONNECT(field) connectField(this, &timer, ui->field, timer.get##field(), &Timer::set##field);

TimerEdit::TimerEdit(Timer &timer, QWidget *parent)
    : QDialog(parent), ui(new Ui::TimerEdit), timer(timer)
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
  QButtonGroup *occurrenceGroup = new QButtonGroup(this);
  occurrenceGroup->setExclusive(true);
  occurrenceGroup->addButton(ui->Occurrence_Interval);
  occurrenceGroup->setId(ui->Occurrence_Interval, (int)Occurrence::Interval);
  occurrenceGroup->addButton(ui->Occurrence_Time);
  occurrenceGroup->setId(ui->Occurrence_Time, (int)Occurrence::Time);
  connect(occurrenceGroup, &QButtonGroup::idClicked, this, &TimerEdit::on_OccurrenceIdClicked);
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

void TimerEdit::on_OccurrenceIdClicked(int id)
{
  timer.setOccurrence((Occurrence)id);
}
