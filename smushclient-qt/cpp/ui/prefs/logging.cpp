#include "logging.h"
#include "ui_logging.h"

PrefsLogging::PrefsLogging(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PrefsLogging)
{
    ui->setupUi(this);
}

PrefsLogging::~PrefsLogging()
{
    delete ui;
}
