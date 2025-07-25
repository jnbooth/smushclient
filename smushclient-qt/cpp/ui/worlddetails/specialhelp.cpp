#include "specialhelp.h"
#include "../../fieldconnector.h"
#include "ui_specialhelp.h"

SpecialHelp::SpecialHelp(QWidget *parent)
    : QDialog(parent), ui(new Ui::SpecialHelp) {
  ui->setupUi(this);
  QPalette palette = ui->textEdit->palette();
  palette.setColor(QPalette::ColorRole::Base,
                   palette.color(QPalette::ColorRole::Window));
  ui->textEdit->setPalette(palette);
}

SpecialHelp::~SpecialHelp() { delete ui; }
