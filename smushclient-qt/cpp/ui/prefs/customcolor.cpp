#include "customcolor.h"
#include "ui_customcolor.h"

PrefsCustomColor::PrefsCustomColor(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PrefsCustomColor)
{
    ui->setupUi(this);
}

PrefsCustomColor::~PrefsCustomColor()
{
    delete ui;
}
