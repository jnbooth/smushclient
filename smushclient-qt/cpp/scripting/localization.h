#include <QtCore/QString>

struct lua_State;

QString formatCompileError(lua_State *L);

QString formatPanic(lua_State *L);

QString formatRuntimeError(lua_State *L);
