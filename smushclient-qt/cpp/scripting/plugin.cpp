#include "plugin.h"
#include "callback/plugincallback.h"
#include "lua/api.h"
#include "lua/errors.h"
#include "lua/init.h"
#include "qlua.h"
#include "scriptapi.h"
#include "scriptthread.h"
#include "smushclient_qt/src/ffi/client.cxxqt.h"
#include <QtCore/QFileInfo>
#include <QtWidgets/QErrorMessage>
extern "C"
{
#include "lauxlib.h"
}

using std::string_view;

// Private utils

namespace {
inline bool
checkError(int status)
{
  switch (status) {
    case LUA_OK:
      return false;
    case LUA_ERRMEM:
      throw std::bad_alloc();
    default:
      return true;
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

Plugin::Plugin(ScriptApi* api, const PluginPack& pack, size_t index)
  : metadata(pack, index)
{
  reset(api);
}

Plugin::Plugin(Plugin&& other) noexcept
  : disabled(other.disabled)
  , L(std::exchange(other.L, nullptr))
  , metadata(std::move(other.metadata))
{
}

Plugin::~Plugin()
{
  if (L != nullptr) {
    lua_close(L);
  }
}

void
Plugin::disable()
{
  disabled = true;
}

void
Plugin::enable()
{
  disabled = false;
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
  if (CallbackFinder(routine).findCallback(L)) {
    lua_pop(L, 1);
    return true;
  }
  return false;
}

bool
Plugin::hasFunction(const QString& routine) const
{
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
  if (pack.scriptSize != 0 && !runScript(script)) {
    disable();
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
    disable();
    return false;
  }
  return true;
}

void
Plugin::reset()
{
  reset(&getApi(L));
}

void
Plugin::reset(ScriptApi* api)
{
  disabled = false;

  if (L != nullptr) {
    lua_State* oldL = L;
    L = nullptr;
    lua_close(oldL);
  }

  L = luaL_newstate();

  if (L == nullptr) {
    throw std::bad_alloc();
  }

  initLuaState(L);
  setPluginIndex(L, metadata.index);
  setLuaApi(L, api);
  lua_settop(L, 0);
}

bool
Plugin::runCallback(PluginCallback& callback) const
{
  if (!findCallback(callback)) {
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
  if (!findCallback(callback)) {
    return false;
  }
  const ScriptThread thread(L);
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

  if (checkError(luaL_loadfile(L, path.toUtf8().data()))) [[unlikely]] {
    getApi(L).printError(formatCompileError(L));
    lua_pop(L, 1);
    return false;
  }

  return api_pcall(L, 0, 0);
}

bool
Plugin::runScript(string_view script) const
{
  if (disabled || script.empty()) [[unlikely]] {
    return false;
  }

  if (checkError(qlua::loadString(L, script))) [[unlikely]] {
    getApi(L).printError(formatCompileError(L));
    lua_pop(L, 1);
    return false;
  }

  return api_pcall(L, 0, 0);
}

void
Plugin::updateMetadata(const PluginPack& pack, size_t index)
{
  metadata = PluginMetadata(pack, index);
}

// Private methods

bool
Plugin::findCallback(const PluginCallback& callback) const
{
  if (disabled) [[unlikely]] {
    return false;
  }

  return callback.findCallback(L);
}
