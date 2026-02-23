#include "scriptthread.h"
#include "lua/errors.h"
extern "C"
{
#include "lua.h"
}

using std::shared_ptr;

// Public methods

ScriptThread::ScriptThread(const shared_ptr<lua_State>& parentL)
  : L(lua_newthread(parentL.get()))
  , parentLptr(parentL)
{
  lua_rawsetp(parentL.get(), LUA_REGISTRYINDEX, L);
  pushErrorHandler(L);
}

ScriptThread::ScriptThread(ScriptThread&& other) noexcept
  : L(std::exchange(other.L, nullptr))
  , parentLptr(std::move(other.parentLptr))
{
}

ScriptThread::~ScriptThread()
{
  if (L == nullptr) {
    return;
  }
  auto parentLock = parentLptr.lock();
  if (!parentLock) {
    return;
  }
  lua_State* parentL = parentLock.get();
  if (parentL == nullptr) {
    return;
  }
  lua_pushnil(parentL);
  lua_rawsetp(parentL, LUA_REGISTRYINDEX, L);
  lua_closethread(L, nullptr);
}
