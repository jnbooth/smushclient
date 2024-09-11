#include "lauxlib.h"

#define luaL_reg luaL_Reg

#define luaL_register(L, name, lib) \
  (luaL_newlib(L, lib), lua_pushvalue(L, -1), lua_setglobal(L, name))
