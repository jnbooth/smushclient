#include "plugin.h"
#include "callback/plugincallback.h"
#include "lua/api.h"
#include "lua/errors.h"
#include "lua/init.h"
#include "scriptapi.h"
#include "scriptthread.h"
#include "smushclient_qt/src/ffi/client.cxxqt.h"
#include <QtCore/QFileInfo>
#include <QtWidgets/QErrorMessage>
#include <memory>
extern "C"
{
#include "lauxlib.h"
}

using std::string_view;

// Private utils

namespace {
inline bool
runLoaded(lua_State* L, int status)
{
  switch (status) {
    case LUA_OK:
      [[likely]] return api_pcall(L, 0, 0);
    case LUA_ERRMEM:
      throw std::bad_alloc();
    default:
      getApi(L).printError(formatCompileError(L));
      lua_pop(L, 1);
      return false;
  }
}
} // namespace

// Metadata

PluginMetadata::PluginMetadata(const PluginPack& pack, size_t index)
  : id(pack.id.data(), pack.id.size())
  , index(index)
  , installed(QDateTime::currentDateTimeUtc())
  , name(pack.name.data(), pack.name.size())
{
}

// Public methods

Plugin::Plugin(ScriptApi& api, const PluginPack& pack, size_t index)
  : metadata(pack, index)
{
  reset(api);
}

class CallbackFinder : public DynamicPluginCallback
{
public:
  explicit CallbackFinder(PluginCallbackKey callback)
    : DynamicPluginCallback(callback)
  {
  }
  explicit CallbackFinder(const QString& callback)
    : DynamicPluginCallback(callback)
  {
  }
  constexpr ActionSource source() const noexcept override
  {
    return ActionSource::Unknown;
  }
};

bool
Plugin::hasFunction(PluginCallbackKey routine) const
{
  lua_State* L = L_.get();
  if (CallbackFinder(routine).findCallback(L)) {
    lua_pop(L, 1);
    return true;
  }
  return false;
}

bool
Plugin::hasFunction(const QString& routine) const
{
  lua_State* L = L_.get();
  if (CallbackFinder(routine).findCallback(L)) {
    lua_pop(L, 1);
    return true;
  }
  return false;
}

bool
Plugin::install(const PluginPack& pack)
{
  string_view script(reinterpret_cast<const char*>(pack.scriptData),
                     pack.scriptSize);
  if (pack.scriptSize != 0 && !runScript(script, pack.path.toUtf8().data())) {
    setEnabled(false);
    return false;
  }
  QString scriptPath = pack.path;
  if (scriptPath.isEmpty()) {
    return true;
  }

  if (!pack.id.empty()) {
    scriptPath.replace(scriptPath.size() - 3, 3, QStringLiteral("lua"));
  }

  const QFileInfo info(scriptPath);
  if (info.isFile() && info.isReadable() && !runFile(scriptPath)) {
    setEnabled(false);
    return false;
  }
  return true;
}

void
Plugin::reset()
{
  lua_State* L = L_.get();
  reset(getApi(L));
}

void
Plugin::reset(ScriptApi& api)
{
  disabled = false;

  L_.reset(luaL_newstate(), lua_close);

  lua_State* L = L_.get();

  if (L == nullptr) {
    throw std::bad_alloc();
  }

  initLuaState(L);
  setPluginIndex(L, metadata.index);
  setLuaApi(L, api);
}

bool
Plugin::runCallback(PluginCallback& callback) const
{
  lua_State* L = L_.get();
  if (disabled || !callback.findCallback(L)) {
    return false;
  }
  if (!api_pcall(L, callback.pushArguments(L), callback.expectedSize())) {
    return false;
  }
  callback.collectReturned(L);
  return true;
}

bool
Plugin::runCallbackThreaded(PluginCallback& callback) const
{
  lua_State* L = L_.get();
  if (disabled || !callback.findCallback(L)) {
    return false;
  }
  const ScriptThread thread(L_);
  lua_State* L2 = thread.state();
  lua_xmove(L, L2, 1);
  if (!api_pcall(L2, callback.pushArguments(L2), callback.expectedSize())) {
    return false;
  }
  callback.collectReturned(L2);
  return true;
}

bool
Plugin::runFile(const QString& path) const
{
  if (disabled) [[unlikely]] {
    return false;
  }

  lua_State* L = L_.get();
  return runLoaded(L, luaL_loadfile(L, path.toUtf8().data()));
}

bool
Plugin::runScript(string_view script, const char* name) const
{
  if (disabled || script.empty()) [[unlikely]] {
    return false;
  }

  lua_State* L = L_.get();
  return runLoaded(L, luaL_loadbuffer(L, script.data(), script.size(), name));
}

void
Plugin::setEnabled(bool enable)
{
  disabled = !enable;
}

ScriptThread
Plugin::spawnThread() const
{
  return ScriptThread(L_);
}

void
Plugin::updateMetadata(const PluginPack& pack, size_t index)
{
  metadata = PluginMetadata(pack, index);
}
