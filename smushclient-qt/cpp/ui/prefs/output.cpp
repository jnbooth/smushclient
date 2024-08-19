#include "output.h"
#include "ui_output.h"

PrefsOutput::PrefsOutput(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PrefsOutput)
{
    ui->setupUi(this);
}

PrefsOutput::~PrefsOutput()
{
    delete ui;
}
