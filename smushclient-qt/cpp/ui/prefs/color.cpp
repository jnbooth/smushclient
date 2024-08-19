#include "color.h"
#include "ui_color.h"

PrefsColor::PrefsColor(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PrefsColor)
{
    ui->setupUi(this);
}

PrefsColor::~PrefsColor()
{
    delete ui;
}
