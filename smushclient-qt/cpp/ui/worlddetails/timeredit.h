#pragma once
#include <QtWidgets/QDialog>
#include "cxx-qt-gen/ffi.cxxqt.h"

namespace Ui
{
  class TimerEdit;
}

class TimerEdit : public QDialog
{
  Q_OBJECT

public:
  explicit TimerEdit(Timer &timer, QWidget *parent = nullptr);
  ~TimerEdit();

  bool groupChanged() const;

private slots:
  void on_OccurrenceChanged(Occurrence value);

  void on_Label_textChanged(const QString &text);
  void on_UserSendTo_currentIndexChanged(int index);
  void on_Text_textChanged();

private:
  Ui::TimerEdit *ui;
  QString originalGroup;
  Timer &timer;
};
