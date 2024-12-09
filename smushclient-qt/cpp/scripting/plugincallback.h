#pragma once
#include <QtCore/QByteArray>
#include "scriptenums.h"
#include "rust/cxx.h"

struct lua_State;
enum class CommandSource : uint8_t;

class PluginCallbackKey
{
public:
  constexpr PluginCallbackKey(std::string_view name) noexcept
      : name(name),
        property()
  {
    const size_t n = name.find('.');
    if (n == std::string_view::npos)
      return;

    property = name.substr(n + 1);
    name = name.substr(n);
  }

  constexpr PluginCallbackKey(const std::string &name) noexcept
      : PluginCallbackKey(std::string_view(name)) {}

  inline PluginCallbackKey(rust::Str name) noexcept
      : PluginCallbackKey(std::string_view(name.data(), name.length())) {}

  constexpr PluginCallbackKey(const rust::String &name) noexcept
      : PluginCallbackKey(std::string_view(name.data(), name.length())) {}

  PluginCallbackKey(const QString &name);

public:
  std::string_view name;
  std::string_view property;
};

constexpr ActionSource commandAction(CommandSource source) noexcept
{
  return (bool)source ? ActionSource::UserTyping : ActionSource::UserKeypad;
}

#define CALLBACK(idNumber, nameString, sourceAction)                                 \
  static const int ID = 1 << idNumber;                                               \
  inline constexpr int id() const noexcept override { return 1 << idNumber; }        \
  inline constexpr const char *name() const noexcept override { return nameString; } \
  inline constexpr ActionSource source() const noexcept override { return sourceAction; }

// Abstract

class PluginCallback
{
public:
  virtual ~PluginCallback() {}

  virtual int id() const noexcept = 0;
  virtual ActionSource source() const noexcept = 0;
  virtual int expectedSize() const noexcept { return 0; }
  virtual int pushArguments(lua_State *) const { return 0; }
  virtual bool findCallback(lua_State *L) const = 0;
  virtual void collectReturned(lua_State *) {}
};

class DynamicPluginCallback : public PluginCallback
{
public:
  constexpr DynamicPluginCallback(PluginCallbackKey name) noexcept
      : name(name.name),
        property(name.property) {}

  virtual ~DynamicPluginCallback() {}

  virtual int id() const noexcept override { return 0; }
  virtual bool findCallback(lua_State *L) const override;

private:
  std::string name;
  std::string property;
};

class NamedPluginCallback : public PluginCallback
{
public:
  virtual ~NamedPluginCallback() {}

  virtual const char *name() const noexcept = 0;
  virtual bool findCallback(lua_State *L) const override;
};

class DiscardCallback : public NamedPluginCallback
{
public:
  inline constexpr int expectedSize() const noexcept override { return 1; }
  constexpr DiscardCallback() : processing(true) {}
  void collectReturned(lua_State *L) override;
  inline constexpr bool discarded() const { return !processing; }

private:
  bool processing;
};

class ModifyTextCallback : public NamedPluginCallback
{
public:
  inline constexpr int expectedSize() const noexcept override { return 1; }
  constexpr ModifyTextCallback(QByteArray &text) : text(text) {}
  void collectReturned(lua_State *L) override;
  int pushArguments(lua_State *L) const override;

private:
  QByteArray &text;
};

// Concrete

class OnPluginBroadcast : public NamedPluginCallback
{
public:
  CALLBACK(0, "OnPluginBroadcast", ActionSource::Unknown)
  constexpr OnPluginBroadcast(
      int message,
      std::string_view pluginID,
      std::string_view pluginName,
      std::string_view text)
      : message(message),
        pluginID(pluginID),
        pluginName(pluginName),
        text(text) {}
  int pushArguments(lua_State *L) const override;

private:
  int message;
  std::string_view pluginID;
  std::string_view pluginName;
  std::string_view text;
};

class OnPluginCommand : public DiscardCallback
{
public:
  CALLBACK(1, "OnPluginCommand", commandAction(commandSource))
  constexpr OnPluginCommand(CommandSource commandSource, const QByteArray &text)
      : DiscardCallback(),
        commandSource(commandSource),
        text(text) {}
  int pushArguments(lua_State *L) const override;

private:
  CommandSource commandSource;
  const QByteArray &text;
};

class OnPluginCommandChanged : public NamedPluginCallback
{
public:
  CALLBACK(2, "OnPluginCommandChanged", ActionSource::UserTyping)
};

class OnPluginClose : public NamedPluginCallback
{
public:
  CALLBACK(3, "OnPluginClose", ActionSource::Unknown)
};

class OnPluginCommandEntered : public ModifyTextCallback
{
public:
  CALLBACK(4, "OnPluginCommandEntered", commandAction(commandSource))
  constexpr OnPluginCommandEntered(CommandSource commandSource, QByteArray &text)
      : ModifyTextCallback(text),
        commandSource(commandSource) {}

private:
  CommandSource commandSource;
};

class OnPluginConnect : public NamedPluginCallback
{
public:
  CALLBACK(5, "OnPluginConnect", ActionSource::WorldAction)
};

class OnPluginDisconnect : public NamedPluginCallback
{
public:
  CALLBACK(6, "OnPluginDisconnect", ActionSource::WorldAction)
};

class OnPluginGetFocus : public NamedPluginCallback
{
public:
  CALLBACK(7, "OnPluginGetFocus", ActionSource::WorldAction)
};

class OnPluginIacGa : public NamedPluginCallback
{
public:
  CALLBACK(8, "OnPlugin_IAC_GA", ActionSource::Unknown)
};

class OnPluginInstall : public NamedPluginCallback
{
public:
  CALLBACK(9, "OnPluginInstall", ActionSource::Unknown)
};

class OnPluginLineReceived : public DiscardCallback
{
public:
  CALLBACK(10, "OnPluginLineReceived", ActionSource::InputFromServer)
  constexpr OnPluginLineReceived(std::string_view line) : DiscardCallback(), line(line) {}
  int pushArguments(lua_State *L) const override;

private:
  std::string_view line;
};

class OnPluginListChanged : public NamedPluginCallback
{
public:
  CALLBACK(11, "OnPluginListChanged", ActionSource::Unknown)
};

class OnPluginLoseFocus : public NamedPluginCallback
{
public:
  CALLBACK(12, "OnPluginLoseFocus", ActionSource::WorldAction)
};

class OnPluginMXPStart : public NamedPluginCallback
{
public:
  CALLBACK(13, "OnPluginMXPstart", ActionSource::WorldAction)
};

class OnPluginMXPStop : public NamedPluginCallback
{
public:
  CALLBACK(14, "OnPluginMXPstop", ActionSource::WorldAction)
};

class OnPluginMXPSetEntity : public NamedPluginCallback
{
public:
  CALLBACK(15, "OnPluginMXPsetEntity", ActionSource::WorldAction)
  constexpr OnPluginMXPSetEntity(std::string_view value) : value(value) {}
  int pushArguments(lua_State *L) const override;

private:
  std::string_view value;
};

class OnPluginMXPSetVariable : public NamedPluginCallback
{
public:
  CALLBACK(16, "OnPluginMXPsetVariable", ActionSource::WorldAction)
  constexpr OnPluginMXPSetVariable(std::string_view variable, std::string_view contents)
      : variable(variable),
        contents(contents) {}
  int pushArguments(lua_State *L) const override;

private:
  std::string_view variable;
  std::string_view contents;
};

class OnPluginSaveState : public NamedPluginCallback
{
public:
  CALLBACK(17, "OnPluginSaveState", ActionSource::Unknown)
};

class OnPluginSend : public DiscardCallback
{
public:
  CALLBACK(18, "OnPluginSend", ActionSource::Unknown)
  constexpr OnPluginSend(const QByteArray &text) : DiscardCallback(), text(text) {}
  int pushArguments(lua_State *L) const override;

private:
  const QByteArray &text;
};

class OnPluginSent : public NamedPluginCallback
{
public:
  CALLBACK(19, "OnPluginSent", ActionSource::Unknown)
  constexpr OnPluginSent(const QByteArray &text) : text(text) {}
  int pushArguments(lua_State *L) const override;

private:
  const QByteArray &text;
};

class OnPluginTabComplete : public ModifyTextCallback
{
public:
  CALLBACK(20, "OnPluginTabComplete", ActionSource::UserTyping)
  constexpr OnPluginTabComplete(QByteArray &text) : ModifyTextCallback(text) {}
};

class OnPluginTelnetRequest : public NamedPluginCallback
{
public:
  CALLBACK(21, "OnPluginTelnetRequest", ActionSource::Unknown)
  constexpr OnPluginTelnetRequest(uint8_t code, std::string_view message) : code(code), message(message) {}
  int pushArguments(lua_State *L) const override;

private:
  int code;
  std::string_view message;
};

class OnPluginTelnetSubnegotiation : public NamedPluginCallback
{
public:
  CALLBACK(22, "OnPluginTelnetSubnegotiation", ActionSource::Unknown)
  constexpr OnPluginTelnetSubnegotiation(uint8_t code, const QByteArray &data) : code(code), data(data) {}
  int pushArguments(lua_State *L) const override;

private:
  int code;
  const QByteArray &data;
};

class OnPluginWorldSave : public NamedPluginCallback
{
public:
  CALLBACK(23, "OnPluginWorldSave", ActionSource::Unknown)
};

class OnPluginWorldOutputResized : public NamedPluginCallback
{
public:
  CALLBACK(24, "OnPluginWorldOutputResized", ActionSource::Unknown)
};

class CallbackFilter
{
public:
  inline void operator|=(const CallbackFilter &other) { filter |= other.filter; }
  inline void clear() { filter = 0; }
  inline constexpr bool includes(const PluginCallback &callback) const
  {
    return filter & callback.id();
  }
  void scan(lua_State *L);

private:
  void setIfDefined(lua_State *L, const NamedPluginCallback &callback);

private:
  size_t filter = 0;
};

#undef CALLBACK
