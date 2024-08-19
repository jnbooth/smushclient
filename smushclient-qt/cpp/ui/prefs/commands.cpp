#include "commands.h"
#include "ui_commands.h"

PrefsCommands::PrefsCommands(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PrefsCommands)
{
    ui->setupUi(this);
}

PrefsCommands::~PrefsCommands()
{
    delete ui;
}
