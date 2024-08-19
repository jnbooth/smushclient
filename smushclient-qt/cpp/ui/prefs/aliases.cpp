#include "aliases.h"
#include "ui_aliases.h"

PrefsAliases::PrefsAliases(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PrefsAliases)
{
    ui->setupUi(this);
}

PrefsAliases::~PrefsAliases()
{
    delete ui;
}
