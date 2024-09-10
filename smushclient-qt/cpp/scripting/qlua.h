#pragma once
#include <QtCore/QString>
#include <QtGui/QColor>

extern "C"
{
#include "lua.h"
}

namespace qlua
{
  int loadQString(lua_State *L, const QString &chunk);
  QColor getQColor(lua_State *L, int idx);
  QString getQString(lua_State *L, int idx);
}
