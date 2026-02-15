#pragma once
#include "../scriptenums.h"
#include "key.h"

struct lua_State;
enum class CommandSource : uint8_t;

#define CALLBACK(idNumber, nameString, sourceAction)                           \
  static const unsigned int ID = 1 << (idNumber);                              \
  inline constexpr unsigned int id() const noexcept override                   \
  {                                                                            \
    return 1 << (idNumber);                                                    \
  }                                                                            \
  inline constexpr const char* name() const noexcept override                  \
  {                                                                            \
    return nameString;                                                         \
  }                                                                            \
  inline constexpr ActionSource source() const noexcept override               \
  {                                                                            \
    return sourceAction;                                                       \
  }

// Abstract

class PluginCallback
{
public:
  PluginCallback() = default;
  virtual ~PluginCallback() = default;

  PluginCallback(const PluginCallback&) = delete;
  PluginCallback& operator=(const PluginCallback&) = delete;
  PluginCallback& operator=(PluginCallback&&) = delete;
  PluginCallback(PluginCallback&& boo) = delete;

  virtual unsigned int id() const noexcept = 0;
  virtual ActionSource source() const noexcept = 0;
  virtual int expectedSize() const noexcept { return 0; }
  virtual int pushArguments(lua_State* /*L*/) const { return 0; }
  virtual bool findCallback(lua_State* L) const = 0;
  virtual void collectReturned(lua_State* /*L*/) {}

  static constexpr ActionSource commandAction(CommandSource source) noexcept
  {
    return static_cast<bool>(source) ? ActionSource::UserTyping
                                     : ActionSource::UserKeypad;
  }
};

class DynamicPluginCallback : public PluginCallback
{
public:
  constexpr explicit DynamicPluginCallback(PluginCallbackKey callback) noexcept
    : name(callback.name)
    , property(callback.property)
  {
  }

  explicit DynamicPluginCallback(const QString& callback);

  unsigned int id() const noexcept override { return 0; }
  bool findCallback(lua_State* L) const override;

private:
  std::string name;
  std::string property;
};

class NamedPluginCallback : public PluginCallback
{
public:
  virtual const char* name() const noexcept = 0;
  bool findCallback(lua_State* L) const override;
};

class DiscardCallback : public NamedPluginCallback
{
public:
  constexpr int expectedSize() const noexcept override { return 1; }
  constexpr DiscardCallback() = default;
  void collectReturned(lua_State* L) override;
  constexpr bool discarded() const { return !processing; }

private:
  bool processing = true;
};

class ModifyTextCallback : public NamedPluginCallback
{
public:
  constexpr int expectedSize() const noexcept override { return 1; }
  explicit constexpr ModifyTextCallback(QByteArray& text)
    : text(text)
  {
  }
  void collectReturned(lua_State* L) override;
  int pushArguments(lua_State* L) const override;

private:
  QByteArray& text;
};

// Concrete

class OnPluginBroadcast : public NamedPluginCallback
{
public:
  CALLBACK(0, "OnPluginBroadcast", ActionSource::Unknown)
  constexpr OnPluginBroadcast(int64_t message,
                              std::string_view pluginID,
                              std::string_view pluginName,
                              std::string_view text)
    : message(message)
    , pluginID(pluginID)
    , pluginName(pluginName)
    , text(text)
  {
  }
  int pushArguments(lua_State* L) const override;

private:
  int64_t message;
  std::string_view pluginID;
  std::string_view pluginName;
  std::string_view text;
};

class OnPluginCommand : public DiscardCallback
{
public:
  CALLBACK(1, "OnPluginCommand", commandAction(commandSource))
  constexpr OnPluginCommand(CommandSource commandSource, const QByteArray& text)
    : commandSource(commandSource)
    , text(text)
  {
  }
  int pushArguments(lua_State* L) const override;

private:
  CommandSource commandSource;
  const QByteArray& text;
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
  constexpr OnPluginCommandEntered(CommandSource commandSource,
                                   QByteArray& text)
    : ModifyTextCallback(text)
    , commandSource(commandSource)
  {
  }

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
  constexpr explicit OnPluginLineReceived(std::string_view line)
    : line(line)
  {
  }
  int pushArguments(lua_State* L) const override;

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
  constexpr explicit OnPluginMXPSetEntity(std::string_view value)
    : value(value)
  {
  }
  int pushArguments(lua_State* L) const override;

private:
  std::string_view value;
};

class OnPluginMXPSetVariable : public NamedPluginCallback
{
public:
  CALLBACK(16, "OnPluginMXPsetVariable", ActionSource::WorldAction)
  constexpr OnPluginMXPSetVariable(std::string_view variable,
                                   std::string_view contents)
    : variable(variable)
    , contents(contents)
  {
  }
  int pushArguments(lua_State* L) const override;

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
  explicit constexpr OnPluginSend(const QByteArray& text)
    : text(text)
  {
  }
  int pushArguments(lua_State* L) const override;

private:
  const QByteArray& text;
};

class OnPluginSent : public NamedPluginCallback
{
public:
  CALLBACK(19, "OnPluginSent", ActionSource::Unknown)
  explicit constexpr OnPluginSent(const QByteArray& text)
    : text(text)
  {
  }
  int pushArguments(lua_State* L) const override;

private:
  const QByteArray& text;
};

class OnPluginTabComplete : public ModifyTextCallback
{
public:
  CALLBACK(20, "OnPluginTabComplete", ActionSource::UserTyping)
  explicit constexpr OnPluginTabComplete(QByteArray& text)
    : ModifyTextCallback(text)
  {
  }
};

class OnPluginTelnetRequest : public NamedPluginCallback
{
public:
  CALLBACK(21, "OnPluginTelnetRequest", ActionSource::Unknown)
  explicit constexpr OnPluginTelnetRequest(uint8_t code,
                                           std::string_view message)
    : code(code)
    , message(message)
  {
  }
  int pushArguments(lua_State* L) const override;

private:
  uint8_t code;
  std::string_view message;
};

class OnPluginTelnetSubnegotiation : public NamedPluginCallback
{
public:
  CALLBACK(22, "OnPluginTelnetSubnegotiation", ActionSource::Unknown)
  constexpr OnPluginTelnetSubnegotiation(uint8_t code, const QByteArray& data)
    : code(code)
    , data(data)
  {
  }
  int pushArguments(lua_State* L) const override;

private:
  uint8_t code;
  const QByteArray& data;
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
#undef CALLBACK

class TimerCallback : public DynamicPluginCallback
{
public:
  TimerCallback(const rust::String& callback, const rust::String& label)
    : DynamicPluginCallback(callback)
    , label(label)
  {
  }

  constexpr ActionSource source() const noexcept override
  {
    return ActionSource::TimerFired;
  }

  int pushArguments(lua_State* L) const override;

private:
  const rust::String& label;
};
