#include "chat.h"
#include "ui_chat.h"

PrefsChat::PrefsChat(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PrefsChat)
{
    ui->setupUi(this);
}

PrefsChat::~PrefsChat()
{
    delete ui;
}
