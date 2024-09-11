#include "lualibs.h"
#include "libbase64.h"
#include "qlua.h"
extern "C"
{
#include "lauxlib.h"
#include "lualib.h"
  LUALIB_API int luaopen_bc(lua_State *L);
  LUALIB_API int luaopen_bit(lua_State *L);
  LUALIB_API int luaopen_lpeg(lua_State *L);
  LUALIB_API int luaopen_rex_pcre2(lua_State *L);
  LUALIB_API int luaopen_lsqlite3(lua_State *L);
}

static int L_base64encode(lua_State *L)
{
  size_t inputLength;
  const char *input = luaL_checklstring(L, 1, &inputLength);
  char *output = new char[inputLength * 4 / 3];
  size_t outputLength;
  base64_encode(input, inputLength, output, &outputLength, 0);
  lua_pushlstring(L, output, outputLength);
  delete[] output;
  return 1;
}

static int L_base64decode(lua_State *L)
{
  size_t inputLength;
  const char *input = luaL_checklstring(L, 1, &inputLength);
  char *output = new char[inputLength * 3 / 4];
  size_t outputLength;
  base64_decode(input, inputLength, output, &outputLength, 0);
  lua_pushlstring(L, output, outputLength);
  delete[] output;
  return 1;
}

static int L_split(lua_State *L)
{
  size_t inputLength;
  const char *input = luaL_checklstring(L, 1, &inputLength);
  size_t sepLength;
  const char *sep = luaL_checklstring(L, 2, &sepLength);
  const int count = qlua::getInt(L, 2, 0);
  if (sepLength != 1)
    luaL_error(L, "Separator must be a single character");
  if (count < 0)
    luaL_error(L, "Count must be positive or zero");

  lua_newtable(L);
  int max = count == 0 ? INT_MAX : count;

  const char *endPtr = input + inputLength;
  const char *sepPtr;
  int i = 1;
  for (; i <= max && input < endPtr && (sepPtr = strchr(input, *sep)) != NULL; i += 1)
  {
    lua_pushlstring(L, input, sepPtr - input);
    lua_rawseti(L, -2, i);
    input = sepPtr + 1;
  }
  if (input < endPtr)
  {
    lua_pushlstring(L, input, endPtr - input);
    lua_rawseti(L, -2, i);
  }
  return 1;
}

static const struct luaL_Reg utillib[] =
    {{"base64decode", L_base64decode},
     {"base64encode", L_base64encode},
     {"split", L_split},

     {NULL, NULL}};

inline void luaopen_util(lua_State *L)
{
  luaL_newlib(L, utillib);
  lua_setglobal(L, "util");
}

void openLuaLibs(lua_State *L)
{
  luaL_openlibs(L);
  luaopen_bc(L);
  luaopen_bit(L);
  luaopen_lpeg(L);
  luaopen_rex_pcre2(L);
  luaopen_lsqlite3(L);
  luaopen_util(L);
  lua_settop(L, 0);
}
