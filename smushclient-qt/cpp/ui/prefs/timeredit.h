#ifndef TIMEREDIT_H
#define TIMEREDIT_H

#include "cxx-qt-gen/ffi.cxxqt.h"
#include <QtWidgets/QDialog>
#include <QtWidgets/QWidget>

namespace Ui
{
  class TimerEdit;
}

class TimerEdit : public QDialog
{
  Q_OBJECT

public:
  explicit TimerEdit(Timer *timer, QWidget *parent = nullptr);
  ~TimerEdit();

private slots:
  void on_Text_textChanged();
  void on_OccurrenceIdClicked(int id);

private:
  Ui::TimerEdit *ui;
  Timer *timer;
};

#endif // TIMEREDIT_H
