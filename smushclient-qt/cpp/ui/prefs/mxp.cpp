#include "mxp.h"
#include "ui_mxp.h"

PrefsMxp::PrefsMxp(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PrefsMxp)
{
    ui->setupUi(this);
}

PrefsMxp::~PrefsMxp()
{
    delete ui;
}
