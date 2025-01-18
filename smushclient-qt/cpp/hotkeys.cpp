#include "hotkeys.h"
#include "smushclient_qt/src/ffi/world.cxxqt.h"

// Private utils

inline QString numpadBase(const World &world, Qt::Key key)
{
  switch (key)
  {
  case Qt::Key::Key_0:
    return world.getNumpad0();
  case Qt::Key::Key_1:
    return world.getNumpad1();
  case Qt::Key::Key_2:
    return world.getNumpad2();
  case Qt::Key::Key_3:
    return world.getNumpad3();
  case Qt::Key::Key_4:
    return world.getNumpad4();
  case Qt::Key::Key_5:
    return world.getNumpad5();
  case Qt::Key::Key_6:
    return world.getNumpad6();
  case Qt::Key::Key_7:
    return world.getNumpad7();
  case Qt::Key::Key_8:
    return world.getNumpad8();
  case Qt::Key::Key_9:
    return world.getNumpad9();
  case Qt::Key::Key_Period:
    return world.getNumpadPeriod();
  case Qt::Key::Key_Slash:
    return world.getNumpadSlash();
  case Qt::Key::Key_Asterisk:
    return world.getNumpadAsterisk();
  case Qt::Key::Key_Minus:
    return world.getNumpadMinus();
  case Qt::Key::Key_Plus:
    return world.getNumpadPlus();
  default:
    return QString();
  }
}

inline QString numpadMod(const World &world, Qt::Key key)
{
  switch (key)
  {
  case Qt::Key::Key_0:
    return world.getNumpadMod0();
  case Qt::Key::Key_1:
    return world.getNumpadMod1();
  case Qt::Key::Key_2:
    return world.getNumpadMod2();
  case Qt::Key::Key_3:
    return world.getNumpadMod3();
  case Qt::Key::Key_4:
    return world.getNumpadMod4();
  case Qt::Key::Key_5:
    return world.getNumpadMod5();
  case Qt::Key::Key_6:
    return world.getNumpadMod6();
  case Qt::Key::Key_7:
    return world.getNumpadMod7();
  case Qt::Key::Key_8:
    return world.getNumpadMod8();
  case Qt::Key::Key_9:
    return world.getNumpadMod9();
  case Qt::Key::Key_Period:
    return world.getNumpadModPeriod();
  case Qt::Key::Key_Slash:
    return world.getNumpadModSlash();
  case Qt::Key::Key_Asterisk:
    return world.getNumpadModAsterisk();
  case Qt::Key::Key_Minus:
    return world.getNumpadModMinus();
  case Qt::Key::Key_Plus:
    return world.getNumpadModPlus();
  default:
    return QString();
  }
}

// Namespace

QString hotkeys::numpad(const World &world, Qt::Key key, bool modified)
{
  return modified ? numpadMod(world, key) : numpadBase(world, key);
}
