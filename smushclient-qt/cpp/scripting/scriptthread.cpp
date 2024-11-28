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
    : L(other.L),
      parentL(other.parentL)
{
  other.moved = true;
}

ScriptThread::~ScriptThread()
{
  if (moved)
    return;
  lua_closethread(L, nullptr);
  lua_pushnil(parentL);
  lua_rawsetp(parentL, LUA_REGISTRYINDEX, L);
}
