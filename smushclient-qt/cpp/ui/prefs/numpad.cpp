#include "numpad.h"
#include "ui_numpad.h"
#include "../../fieldconnector.h"

PrefsNumpad::PrefsNumpad(const World &world, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::PrefsNumpad)
{
  ui->setupUi(this);
  CONNECT_WORLD(Numpad0);
  CONNECT_WORLD(Numpad1);
  CONNECT_WORLD(Numpad2);
  CONNECT_WORLD(Numpad3);
  CONNECT_WORLD(Numpad4);
  CONNECT_WORLD(Numpad5);
  CONNECT_WORLD(Numpad6);
  CONNECT_WORLD(Numpad7);
  CONNECT_WORLD(Numpad8);
  CONNECT_WORLD(Numpad9);
  CONNECT_WORLD(NumpadPeriod);
  CONNECT_WORLD(NumpadSlash);
  CONNECT_WORLD(NumpadAsterisk);
  CONNECT_WORLD(NumpadMinus);
  CONNECT_WORLD(NumpadPlus);
  CONNECT_WORLD(NumpadMod0);
  CONNECT_WORLD(NumpadMod1);
  CONNECT_WORLD(NumpadMod2);
  CONNECT_WORLD(NumpadMod3);
  CONNECT_WORLD(NumpadMod4);
  CONNECT_WORLD(NumpadMod5);
  CONNECT_WORLD(NumpadMod6);
  CONNECT_WORLD(NumpadMod7);
  CONNECT_WORLD(NumpadMod8);
  CONNECT_WORLD(NumpadMod9);
  CONNECT_WORLD(NumpadModPeriod);
  CONNECT_WORLD(NumpadModSlash);
  CONNECT_WORLD(NumpadModAsterisk);
  CONNECT_WORLD(NumpadModMinus);
  CONNECT_WORLD(NumpadModPlus);
  CONNECT_WORLD(NumpadEnable);
  CONNECT_WORLD(HotkeyAddsToCommandHistory);
  CONNECT_WORLD(EchoHotkeyInOutputWindow);
}

PrefsNumpad::~PrefsNumpad()
{
  delete ui;
}
