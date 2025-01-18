#include "plugin.h"
#include <QtCore/QFileInfo>
#include <QtWidgets/QErrorMessage>
#include "errors.h"
#include "luaapi.h"
#include "luaglobals.h"
#include "qlua.h"
#include "scriptapi.h"
#include "scriptthread.h"
#include "smushclient_qt/src/ffi/client.cxxqt.h"
extern "C"
{
#include "lauxlib.h"
#include "lualib.h"
  LUALIB_API int luaopen_bc(lua_State *L);
  LUALIB_API int luaopen_bit(lua_State *L);
  LUALIB_API int luaopen_cjson(lua_State *L);
  LUALIB_API int luaopen_lpeg(lua_State *L);
  LUALIB_API int luaopen_rex_pcre2(lua_State *L);
  LUALIB_API int luaopen_lsqlite3(lua_State *L);
}
int luaopen_utils(lua_State *L);

using std::string;
using std::string_view;

// Private Lua functions

static int L_panic(lua_State *L)
{
  const QString message = formatPanic(L);
  qCritical() << "panic(" << message << ")";
  QErrorMessage::qtHandler()->showMessage(message);
  return 0;
}

static int L_print(lua_State *L)
{
  const QString output = qlua::concatStrings(L);
#ifdef NDEBUG
  getApi(L).Tell(output);
#endif
  qInfo() << "print(" << output << ")";
  return 0;
}

// Private utils

inline bool checkError(int status)
{
  switch (status)
  {
  case LUA_OK:
    return false;
  case LUA_ERRMEM:
    throw std::bad_alloc();
  default:
    return true;
  }
}

void setlib(lua_State *L, const char *name)
{
  lua_pushvalue(L, -1);
  lua_setglobal(L, name);
  lua_setfield(L, 1, name);
}

// Metadata

PluginMetadata::PluginMetadata(const PluginPack &pack, size_t index)
    : id(pack.id.toStdString()),
      index(index),
      installed(QDateTime::currentDateTimeUtc()),
      name(pack.name.toStdString()) {}

// Public methods

Plugin::Plugin(ScriptApi *api, const PluginPack &pack, size_t index)
    : metadata(pack, index)
{
  reset(api);
}

Plugin::Plugin(Plugin &&other)
    : metadata(std::move(other.metadata)),
      L(std::exchange(other.L, nullptr)),
      isDisabled(other.isDisabled) {}

Plugin::~Plugin()
{
  if (L)
    lua_close(L);
}

void Plugin::disable()
{
  isDisabled = true;
}

void Plugin::enable()
{
  isDisabled = false;
}

class CallbackFinder : public DynamicPluginCallback
{
public:
  inline CallbackFinder(PluginCallbackKey callback) : DynamicPluginCallback(callback) {}
  inline CallbackFinder(const QString &callback) : DynamicPluginCallback(callback) {}
  inline constexpr ActionSource source() const noexcept override { return ActionSource::Unknown; }
};

bool Plugin::hasFunction(PluginCallbackKey routine) const
{
  if (CallbackFinder(routine).findCallback(L))
  {
    lua_pop(L, 1);
    return true;
  }
  return false;
}

bool Plugin::hasFunction(const QString &routine) const
{
  if (CallbackFinder(routine).findCallback(L))
  {
    lua_pop(L, 1);
    return true;
  }
  return false;
}

bool Plugin::install(const PluginPack &pack)
{
  if (pack.scriptSize && !runScript(string_view(reinterpret_cast<const char *>(pack.scriptData), pack.scriptSize)))
  {
    disable();
    return false;
  }
  QString scriptPath = pack.path;
  if (scriptPath.isEmpty())
    return true;

  if (!pack.id.isEmpty())
    scriptPath.replace(scriptPath.size() - 3, 3, QStringLiteral("lua"));

  const QFileInfo info(scriptPath);
  if (info.isFile() && info.isReadable() && !runFile(scriptPath))
  {
    disable();
    return false;
  }
  return true;
}

void Plugin::reset()
{
  reset(&getApi(L));
}

void Plugin::reset(ScriptApi *api)
{
  isDisabled = false;

  if (L)
  {
    lua_State *oldL = L;
    L = nullptr;
    lua_close(oldL);
  }

  L = luaL_newstate();

  if (!L)
    throw std::bad_alloc();

  lua_atpanic(L, &L_panic);
  lua_pushcfunction(L, L_print);
  lua_setglobal(L, "print");
  luaL_openlibs(L);
  lua_settop(L, 0);
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  luaopen_bc(L);
  setlib(L, "bc");
  luaopen_bit(L);
  setlib(L, "bit");
  luaopen_cjson(L);
  setlib(L, "cjson");
  luaopen_lpeg(L);
  setlib(L, "lpeg");
  luaopen_rex_pcre2(L);
  setlib(L, "rex");
  luaopen_lsqlite3(L);
  setlib(L, "sqlite3");
  luaopen_utils(L);
  setlib(L, "utils");
  registerLuaGlobals(L);
  registerLuaWorld(L);
  setPluginIndex(L, metadata.index);
  setLuaApi(L, api);
  lua_settop(L, 0);
  addErrorHandler(L);
}

bool Plugin::runCallback(PluginCallback &callback) const
{
  if (!findCallback(callback))
    return false;
  if (!api_pcall(L, callback.pushArguments(L), callback.expectedSize()))
    return false;
  callback.collectReturned(L);
  return true;
}

bool Plugin::runCallbackThreaded(PluginCallback &callback) const
{
  if (!findCallback(callback))
    return false;
  const ScriptThread thread(L);
  lua_State *L2 = thread.state();
  lua_xmove(L, L2, 1);
  if (!api_pcall(L2, callback.pushArguments(L2), callback.expectedSize()))
    return false;
  callback.collectReturned(L2);
  return true;
}

bool Plugin::runFile(const QString &string) const
{
  if (isDisabled) [[unlikely]]
    return false;

  if (checkError(luaL_loadfile(L, string.toUtf8().data()))) [[unlikely]]
  {
    getApi(L).printError(formatCompileError(L));
    lua_pop(L, 1);
    return false;
  }

  return api_pcall(L, 0, 0);
}

bool Plugin::runScript(const QString &script) const
{
  if (isDisabled || script.isEmpty()) [[unlikely]]
    return false;

  if (checkError(qlua::loadQString(L, script))) [[unlikely]]
  {
    getApi(L).printError(formatCompileError(L));
    lua_pop(L, 1);
    return false;
  }

  return api_pcall(L, 0, 0);
}

bool Plugin::runScript(string_view script) const
{
  if (isDisabled || script.empty()) [[unlikely]]
    return false;

  if (checkError(qlua::loadString(L, script))) [[unlikely]]
  {
    getApi(L).printError(formatCompileError(L));
    lua_pop(L, 1);
    return false;
  }

  return api_pcall(L, 0, 0);
}

// Private methods

bool Plugin::findCallback(const PluginCallback &callback) const
{
  if (isDisabled) [[unlikely]]
    return false;

  return callback.findCallback(L);
}
