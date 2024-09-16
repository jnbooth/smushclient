#pragma once
#include <QtWidgets/QDialog>
#include "cxx-qt-gen/ffi.cxxqt.h"

namespace Ui
{
  class TriggerEdit;
}

class TriggerEdit : public QDialog
{
  Q_OBJECT

public:
  explicit TriggerEdit(Trigger &trigger, QWidget *parent = nullptr);
  ~TriggerEdit();

private slots:
  void on_Text_textChanged();

private:
  Ui::TriggerEdit *ui;
  Trigger &trigger;
};
