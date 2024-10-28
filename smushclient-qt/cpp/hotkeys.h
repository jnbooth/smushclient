#pragma once
#include <QtCore/QString>
#include <QtGui/QKeyEvent>

class World;

namespace hotkeys
{
  QString numpad(const World &world, Qt::Key key, bool modified = false);
};
