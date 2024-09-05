#include "specialhelp.h"
#include "ui_specialhelp.h"
#include "../../fieldconnector.h"

SpecialHelp::SpecialHelp(QWidget *parent)
    : QDialog(parent), ui(new Ui::SpecialHelp)
{
  ui->setupUi(this);
}

SpecialHelp::~SpecialHelp()
{
  delete ui;
}
