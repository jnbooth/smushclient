#include "hotkeys.h"
#include "smushclient_qt/src/ffi/world.cxxqt.h"

// Public methods

QString
Keypad::get(Qt::Key key) const noexcept
{
  switch (key) {
    case Qt::Key::Key_0:
      return key0;
    case Qt::Key::Key_1:
      return key1;
    case Qt::Key::Key_2:
      return key2;
    case Qt::Key::Key_3:
      return key3;
    case Qt::Key::Key_4:
      return key4;
    case Qt::Key::Key_5:
      return key5;
    case Qt::Key::Key_6:
      return key6;
    case Qt::Key::Key_7:
      return key7;
    case Qt::Key::Key_8:
      return key8;
    case Qt::Key::Key_9:
      return key9;
    case Qt::Key::Key_Period:
      return keyPeriod;
    case Qt::Key::Key_Slash:
      return keySlash;
    case Qt::Key::Key_Asterisk:
      return keyAsterisk;
    case Qt::Key::Key_Minus:
      return keyMinus;
    case Qt::Key::Key_Plus:
      return keyPlus;
    default:
      return QString();
  }
}

void
Hotkeys::applyWorld(const World& world)
{
  normal.key0 = world.getNumpad0();
  normal.key1 = world.getNumpad1();
  normal.key2 = world.getNumpad2();
  normal.key3 = world.getNumpad3();
  normal.key4 = world.getNumpad4();
  normal.key5 = world.getNumpad5();
  normal.key6 = world.getNumpad6();
  normal.key7 = world.getNumpad7();
  normal.key8 = world.getNumpad8();
  normal.key9 = world.getNumpad9();
  normal.keyPeriod = world.getNumpadPeriod();
  normal.keySlash = world.getNumpadSlash();
  normal.keyAsterisk = world.getNumpadAsterisk();
  normal.keyMinus = world.getNumpadMinus();
  normal.keyPlus = world.getNumpadPlus();

  modified.key0 = world.getNumpadMod0();
  modified.key1 = world.getNumpadMod1();
  modified.key2 = world.getNumpadMod2();
  modified.key3 = world.getNumpadMod3();
  modified.key4 = world.getNumpadMod4();
  modified.key5 = world.getNumpadMod5();
  modified.key6 = world.getNumpadMod6();
  modified.key7 = world.getNumpadMod7();
  modified.key8 = world.getNumpadMod8();
  modified.key9 = world.getNumpadMod9();
  modified.keyPeriod = world.getNumpadModPeriod();
  modified.keySlash = world.getNumpadModSlash();
  modified.keyAsterisk = world.getNumpadModAsterisk();
  modified.keyMinus = world.getNumpadModMinus();
  modified.keyPlus = world.getNumpadModPlus();
}

QString
Hotkeys::get(Qt::Key key, bool isModified) const noexcept
{
  return isModified ? modified.get(key) : normal.get(key);
}
