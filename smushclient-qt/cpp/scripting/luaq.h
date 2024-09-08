#ifndef LUAQ_H
#define LUAQ_H

#include <QtCore/QString>

extern "C"
{
#include "lauxlib.h"
}

int luaQ_loadqstring(lua_State *L, const QString &chunk);
QString luaQ_toqstring(lua_State *L, int idx);

#endif // LUAQ_H
