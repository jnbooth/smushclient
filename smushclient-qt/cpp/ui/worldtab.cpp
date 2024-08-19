#include "worldtab.h"
#include "ui_worldtab.h"

WorldTab::WorldTab(QWidget *parent)
    : QSplitter(parent)
    , ui(new Ui::WorldTab)
{
    ui->setupUi(this);
}

WorldTab::~WorldTab()
{
    delete ui;
}
