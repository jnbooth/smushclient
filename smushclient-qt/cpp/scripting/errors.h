#pragma once
#include <QtCore/QString>

struct lua_State;

QString formatCompileError(lua_State *L);

QString formatPanic(lua_State *L);

QString formatRuntimeError(lua_State *L);

bool api_pcall(lua_State *L, int nargs, int nreturn);

int addErrorHandler(lua_State *L);

int pushErrorHandler(lua_State *L);
