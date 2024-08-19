#include "timers.h"
#include "ui_timers.h"

PrefsTimers::PrefsTimers(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PrefsTimers)
{
    ui->setupUi(this);
}

PrefsTimers::~PrefsTimers()
{
    delete ui;
}
