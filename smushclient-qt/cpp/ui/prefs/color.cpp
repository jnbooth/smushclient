#include "color.h"
#include "ui_color.h"
#include "../../fieldconnector.h"

PrefsColor::PrefsColor(const World &world, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::PrefsColor)
{
  ui->setupUi(this);
  CONNECT_WORLD(UseDefaultColours);
  CONNECT_WORLD(Ansi0);
  CONNECT_WORLD(Ansi1);
  CONNECT_WORLD(Ansi2);
  CONNECT_WORLD(Ansi3);
  CONNECT_WORLD(Ansi4);
  CONNECT_WORLD(Ansi5);
  CONNECT_WORLD(Ansi6);
  CONNECT_WORLD(Ansi7);
  CONNECT_WORLD(Ansi8);
  CONNECT_WORLD(Ansi9);
  CONNECT_WORLD(Ansi10);
  CONNECT_WORLD(Ansi11);
  CONNECT_WORLD(Ansi12);
  CONNECT_WORLD(Ansi13);
  CONNECT_WORLD(Ansi14);
  CONNECT_WORLD(Ansi15);
}

PrefsColor::~PrefsColor()
{
  delete ui;
}
