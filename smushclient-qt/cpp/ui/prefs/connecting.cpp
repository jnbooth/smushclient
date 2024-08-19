#include "connecting.h"
#include "ui_connecting.h"

PrefsConnecting::PrefsConnecting(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PrefsConnecting)
{
    ui->setupUi(this);
}

PrefsConnecting::~PrefsConnecting()
{
    delete ui;
}
