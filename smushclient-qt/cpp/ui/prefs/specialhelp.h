#ifndef SPECIALHELP_H
#define SPECIALHELP_H

#include <QtWidgets/QDialog>
#include <QtWidgets/QWidget>

namespace Ui
{
  class SpecialHelp;
}

class SpecialHelp : public QDialog
{
  Q_OBJECT

public:
  explicit SpecialHelp(QWidget *parent = nullptr);
  ~SpecialHelp();

private:
  Ui::SpecialHelp *ui;
};

#endif // SPECIALHELP_H
