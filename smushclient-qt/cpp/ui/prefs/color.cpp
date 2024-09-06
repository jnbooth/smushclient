#include "color.h"
#include "ui_color.h"
#include "../../fieldconnector.h"

PrefsColor::PrefsColor(const World &world, QWidget *parent)
    : QWidget(parent), ui(new Ui::PrefsColor)
{
  ui->setupUi(this);
  CONNECT_WORLD(UseDefaultColors);
  CONNECT_WORLD(AnsiColors0);
  CONNECT_WORLD(AnsiColors1);
  CONNECT_WORLD(AnsiColors2);
  CONNECT_WORLD(AnsiColors3);
  CONNECT_WORLD(AnsiColors4);
  CONNECT_WORLD(AnsiColors5);
  CONNECT_WORLD(AnsiColors6);
  CONNECT_WORLD(AnsiColors7);
  CONNECT_WORLD(AnsiColors8);
  CONNECT_WORLD(AnsiColors9);
  CONNECT_WORLD(AnsiColors10);
  CONNECT_WORLD(AnsiColors11);
  CONNECT_WORLD(AnsiColors12);
  CONNECT_WORLD(AnsiColors13);
  CONNECT_WORLD(AnsiColors14);
  CONNECT_WORLD(AnsiColors15);
}

PrefsColor::~PrefsColor()
{
  delete ui;
}
