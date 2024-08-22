#include "customcolor.h"
#include "ui_customcolor.h"

PrefsCustomColor::PrefsCustomColor(const World *world, QWidget *parent)
    : AbstractPrefsPane(parent), ui(new Ui::PrefsCustomColor)
{
  ui->setupUi(this);
  CONNECT_WORLD(CustomNames0);
  CONNECT_WORLD(CustomColorsForeground0);
  CONNECT_WORLD(CustomColorsBackground0);

  CONNECT_WORLD(CustomNames1);
  CONNECT_WORLD(CustomColorsForeground1);
  CONNECT_WORLD(CustomColorsBackground1);

  CONNECT_WORLD(CustomNames2);
  CONNECT_WORLD(CustomColorsForeground2);
  CONNECT_WORLD(CustomColorsBackground2);

  CONNECT_WORLD(CustomNames3);
  CONNECT_WORLD(CustomColorsForeground3);
  CONNECT_WORLD(CustomColorsBackground3);

  CONNECT_WORLD(CustomNames4);
  CONNECT_WORLD(CustomColorsForeground4);
  CONNECT_WORLD(CustomColorsBackground4);

  CONNECT_WORLD(CustomNames5);
  CONNECT_WORLD(CustomColorsForeground5);
  CONNECT_WORLD(CustomColorsBackground5);

  CONNECT_WORLD(CustomNames6);
  CONNECT_WORLD(CustomColorsForeground6);
  CONNECT_WORLD(CustomColorsBackground6);

  CONNECT_WORLD(CustomNames7);
  CONNECT_WORLD(CustomColorsForeground7);
  CONNECT_WORLD(CustomColorsBackground7);

  CONNECT_WORLD(CustomNames8);
  CONNECT_WORLD(CustomColorsForeground8);
  CONNECT_WORLD(CustomColorsBackground8);

  CONNECT_WORLD(CustomNames9);
  CONNECT_WORLD(CustomColorsForeground9);
  CONNECT_WORLD(CustomColorsBackground9);

  CONNECT_WORLD(CustomNames10);
  CONNECT_WORLD(CustomColorsForeground10);
  CONNECT_WORLD(CustomColorsBackground10);

  CONNECT_WORLD(CustomNames11);
  CONNECT_WORLD(CustomColorsForeground11);
  CONNECT_WORLD(CustomColorsBackground11);

  CONNECT_WORLD(CustomNames12);
  CONNECT_WORLD(CustomColorsForeground12);
  CONNECT_WORLD(CustomColorsBackground12);

  CONNECT_WORLD(CustomNames13);
  CONNECT_WORLD(CustomColorsForeground13);
  CONNECT_WORLD(CustomColorsBackground13);

  CONNECT_WORLD(CustomNames14);
  CONNECT_WORLD(CustomColorsForeground14);
  CONNECT_WORLD(CustomColorsBackground14);

  CONNECT_WORLD(CustomNames15);
  CONNECT_WORLD(CustomColorsForeground15);
  CONNECT_WORLD(CustomColorsBackground15);
}

PrefsCustomColor::~PrefsCustomColor()
{
  delete ui;
}
