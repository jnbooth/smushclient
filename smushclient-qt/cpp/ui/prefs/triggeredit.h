#ifndef TRIGGEREDIT_H
#define TRIGGEREDIT_H

#include <QtWidgets/QDialog>
#include <QtWidgets/QWidget>

namespace Ui
{
  class TriggerEdit;
}

class TriggerEdit : public QDialog
{
  Q_OBJECT

public:
  explicit TriggerEdit(QWidget *parent = nullptr);
  ~TriggerEdit();

private slots:
  void on_Text_textChanged();

private:
  Ui::TriggerEdit *ui;
};

#endif // TRIGGEREDIT_H
