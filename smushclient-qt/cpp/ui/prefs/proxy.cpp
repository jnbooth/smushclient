#include "proxy.h"
#include "ui_proxy.h"

PrefsProxy::PrefsProxy(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PrefsProxy)
{
    ui->setupUi(this);
}

PrefsProxy::~PrefsProxy()
{
    delete ui;
}
