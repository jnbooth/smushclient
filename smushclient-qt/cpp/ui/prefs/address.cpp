#include "address.h"
#include "ui_address.h"

PrefsAddress::PrefsAddress(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PrefsAddress)
{
    ui->setupUi(this);
}

PrefsAddress::~PrefsAddress()
{
    delete ui;
}
