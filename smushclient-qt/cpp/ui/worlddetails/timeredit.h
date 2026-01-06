#pragma once
#include <QtWidgets/QDialog>

namespace Ui {
class TimerEdit;
}

enum class Occurrence;
class Timer;

class TimerEdit : public QDialog
{
  Q_OBJECT

public:
  explicit TimerEdit(Timer& timer, QWidget* parent = nullptr);
  ~TimerEdit();

private slots:
  void on_OccurrenceChanged(Occurrence value);

  void on_Label_textChanged(const QString& text);
  void on_UserSendTo_currentIndexChanged(int index);
  void on_Text_textChanged();

private:
  Ui::TimerEdit* ui;
  Timer& timer;
};
