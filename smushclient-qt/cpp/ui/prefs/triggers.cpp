#include "triggers.h"
#include "ui_triggers.h"

PrefsTriggers::PrefsTriggers(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PrefsTriggers)
{
    ui->setupUi(this);
}

PrefsTriggers::~PrefsTriggers()
{
    delete ui;
}
