#pragma once
#include "../scriptenums.h"
#include "key.h"

struct lua_State;

#define CALLBACK(idNumber, nameString, sourceAction)                           \
  static const unsigned int ID = 1 << (idNumber);                              \
  constexpr unsigned int id() const noexcept override                          \
  {                                                                            \
    return 1 << (idNumber);                                                    \
  }                                                                            \
  constexpr const char* name() const noexcept override                         \
  {                                                                            \
    return (nameString);                                                       \
  }                                                                            \
  constexpr ActionSource source() const noexcept override                      \
  {                                                                            \
    return (sourceAction);                                                     \
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
  constexpr unsigned int id() const noexcept override { return 0; }
  bool findCallback(lua_State* L) const override;

private:
  std::string name;
  std::string property;
};

class NamedPluginCallback : public PluginCallback
{
public:
  static std::span<const NamedPluginCallback* const> list();

public:
  virtual const char* name() const noexcept = 0;
  bool findCallback(lua_State* L) const override;
};

class DiscardCallback : public NamedPluginCallback
{
public:
  constexpr int expectedSize() const noexcept override { return 1; }
  void collectReturned(lua_State* L) override;
  constexpr bool discarded() const noexcept { return !processing; }

private:
  bool processing = true;
};

class ModifyTextCallback : public NamedPluginCallback
{
public:
  explicit constexpr ModifyTextCallback(QByteArray& text) noexcept
    : text(text)
  {
  }
  constexpr int expectedSize() const noexcept override { return 1; }
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
                              QByteArrayView pluginID,
                              QByteArrayView pluginName,
                              QByteArrayView text) noexcept
    : message(message)
    , pluginID(pluginID)
    , pluginName(pluginName)
    , text(text)
  {
  }
  int pushArguments(lua_State* L) const override;

private:
  int64_t message;
  QByteArrayView pluginID;
  QByteArrayView pluginName;
  QByteArrayView text;
};

class OnPluginClose : public NamedPluginCallback
{
public:
  CALLBACK(1, "OnPluginClose", ActionSource::Unknown)
};

class OnPluginCommand : public DiscardCallback
{
public:
  CALLBACK(2, "OnPluginCommand", actionSource)
  constexpr OnPluginCommand(ActionSource actionSource,
                            QByteArrayView text) noexcept
    : actionSource(actionSource)
    , text(text)
  {
  }
  int pushArguments(lua_State* L) const override;

private:
  ActionSource actionSource;
  QByteArrayView text;
};

class OnPluginCommandChanged : public NamedPluginCallback
{
public:
  CALLBACK(3, "OnPluginCommandChanged", ActionSource::UserTyping)
};

class OnPluginCommandEntered : public ModifyTextCallback
{
public:
  CALLBACK(4, "OnPluginCommandEntered", actionSource)
  constexpr OnPluginCommandEntered(ActionSource actionSource,
                                   QByteArray& text) noexcept
    : ModifyTextCallback(text)
    , actionSource(actionSource)
  {
  }

private:
  ActionSource actionSource;
};

class OnPluginConnect : public NamedPluginCallback
{
public:
  CALLBACK(5, "OnPluginConnect", ActionSource::WorldAction)
};

class OnPluginDisable : public NamedPluginCallback
{
public:
  CALLBACK(6, "OnPluginDisable", ActionSource::Unknown)
};

class OnPluginDisconnect : public NamedPluginCallback
{
public:
  CALLBACK(7, "OnPluginDisconnect", ActionSource::WorldAction)
};

class OnPluginEnable : public NamedPluginCallback
{
public:
  CALLBACK(8, "OnPluginEnable", ActionSource::Unknown)
};

class OnPluginGetFocus : public NamedPluginCallback
{
public:
  CALLBACK(9, "OnPluginGetFocus", ActionSource::WorldAction)
};

class OnPluginIacGa : public NamedPluginCallback
{
public:
  CALLBACK(10, "OnPlugin_IAC_GA", ActionSource::Unknown)
};

class OnPluginInstall : public NamedPluginCallback
{
public:
  CALLBACK(11, "OnPluginInstall", ActionSource::Unknown)
};

class OnPluginLineReceived : public DiscardCallback
{
public:
  CALLBACK(12, "OnPluginLineReceived", ActionSource::InputFromServer)
  constexpr explicit OnPluginLineReceived(QByteArrayView line)
    : line(line)
  {
  }
  int pushArguments(lua_State* L) const override;

private:
  QByteArrayView line;
};

class OnPluginListChanged : public NamedPluginCallback
{
public:
  CALLBACK(13, "OnPluginListChanged", ActionSource::Unknown)
};

class OnPluginLoseFocus : public NamedPluginCallback
{
public:
  CALLBACK(14, "OnPluginLoseFocus", ActionSource::WorldAction)
};

class OnPluginMXPSetEntity : public NamedPluginCallback
{
public:
  CALLBACK(15, "OnPluginMXPsetEntity", ActionSource::WorldAction)
  constexpr explicit OnPluginMXPSetEntity(QByteArrayView value) noexcept
    : value(value)
  {
  }
  int pushArguments(lua_State* L) const override;

private:
  QByteArrayView value;
};

class OnPluginMXPSetVariable : public NamedPluginCallback
{
public:
  CALLBACK(16, "OnPluginMXPsetVariable", ActionSource::WorldAction)
  constexpr OnPluginMXPSetVariable(QByteArrayView variable,
                                   QByteArrayView contents) noexcept
    : variable(variable)
    , contents(contents)
  {
  }
  int pushArguments(lua_State* L) const override;

private:
  QByteArrayView variable;
  QByteArrayView contents;
};

class OnPluginMXPStart : public NamedPluginCallback
{
public:
  CALLBACK(17, "OnPluginMXPstart", ActionSource::WorldAction)
};

class OnPluginMXPStop : public NamedPluginCallback
{
public:
  CALLBACK(18, "OnPluginMXPstop", ActionSource::WorldAction)
};

class OnPluginPartialLineReceived : public DiscardCallback
{
public:
  CALLBACK(19, "OnPluginLinePartialReceived", ActionSource::InputFromServer)
  constexpr explicit OnPluginPartialLineReceived(QByteArrayView line)
    : line(line)
  {
  }
  int pushArguments(lua_State* L) const override;

private:
  QByteArrayView line;
};

class OnPluginPlaySound : public DiscardCallback
{
public:
  CALLBACK(20, "OnPluginPlaySound", ActionSource::Unknown)
  constexpr explicit OnPluginPlaySound(QByteArrayView file)
    : file(file)
  {
  }
  int pushArguments(lua_State* L) const override;

private:
  QByteArrayView file;
};

class OnPluginSaveState : public NamedPluginCallback
{
public:
  CALLBACK(21, "OnPluginSaveState", ActionSource::Unknown)
};

class OnPluginSelectionChanged : public NamedPluginCallback
{
public:
  CALLBACK(22, "OnPluginSelectionChanged", ActionSource::Unknown)
};

class OnPluginSend : public DiscardCallback
{
public:
  CALLBACK(23, "OnPluginSend", ActionSource::Unknown)
  explicit constexpr OnPluginSend(QByteArrayView text) noexcept
    : text(text)
  {
  }
  int pushArguments(lua_State* L) const override;

private:
  QByteArrayView text;
};

class OnPluginSent : public NamedPluginCallback
{
public:
  CALLBACK(24, "OnPluginSent", ActionSource::Unknown)
  explicit constexpr OnPluginSent(QByteArrayView text) noexcept
    : text(text)
  {
  }
  int pushArguments(lua_State* L) const override;

private:
  QByteArrayView text;
};

class OnPluginTelnetOption : public NamedPluginCallback
{
public:
  CALLBACK(25, "OnPluginTelnetOption", ActionSource::Unknown)
  explicit constexpr OnPluginTelnetOption(QByteArrayView message) noexcept
    : message(message)
  {
  }
  int pushArguments(lua_State* L) const override;

private:
  QByteArrayView message;
};

class OnPluginTelnetRequest : public NamedPluginCallback
{
public:
  CALLBACK(26, "OnPluginTelnetRequest", ActionSource::Unknown)
  explicit constexpr OnPluginTelnetRequest(uint8_t code,
                                           QByteArrayView message) noexcept
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
  CALLBACK(27, "OnPluginTelnetSubnegotiation", ActionSource::Unknown)
  constexpr OnPluginTelnetSubnegotiation(uint8_t code,
                                         QByteArrayView data) noexcept
    : code(code)
    , data(data)
  {
  }
  int pushArguments(lua_State* L) const override;

private:
  uint8_t code;
  QByteArrayView data;
};

class OnPluginWorldSave : public NamedPluginCallback
{
public:
  CALLBACK(28, "OnPluginWorldSave", ActionSource::Unknown)
};

class OnPluginWorldOutputResized : public NamedPluginCallback
{
public:
  CALLBACK(29, "OnPluginWorldOutputResized", ActionSource::Unknown)
};
#undef CALLBACK

class TimerCallback : public DynamicPluginCallback
{
public:
  TimerCallback(PluginCallbackKey callback, QByteArrayView label) noexcept
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
  const QByteArrayView label;
};
