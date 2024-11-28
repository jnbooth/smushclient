#include "errors.h"
#include "scriptthread.h"
extern "C"
{
#include "lua.h"
}

// Public methods

ScriptThread::ScriptThread(lua_State *parentL)
    : L(lua_newthread(parentL)),
      parentL(parentL)
{
  lua_rawsetp(parentL, LUA_REGISTRYINDEX, L);
  pushErrorHandler(L);
}

ScriptThread::ScriptThread(ScriptThread &&other)
    : L(std::exchange(other.L, nullptr)),
      parentL(std::exchange(other.parentL, nullptr)) {}

ScriptThread::~ScriptThread()
{
  if (L)
    lua_closethread(L, nullptr);
  if (!parentL)
    return;
  lua_pushnil(parentL);
  lua_rawsetp(parentL, LUA_REGISTRYINDEX, L);
}
