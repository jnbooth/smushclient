#pragma once
#include <QtCore/QByteArray>
#include "scriptenums.h"

struct lua_State;

#define CALLBACK(idNumber, nameString, sourceAction)                                 \
  static const int ID = 1 << idNumber;                                               \
  inline constexpr int id() const noexcept override { return 1 << idNumber; }        \
  inline constexpr const char *name() const noexcept override { return nameString; } \
  inline constexpr ActionSource source() const noexcept override { return sourceAction; }

// Abstract

class PluginCallback
{
public:
  virtual const char *name() const noexcept = 0;
  virtual const char *property() const noexcept { return nullptr; }
  virtual int id() const noexcept { return 0; }
  virtual ActionSource source() const noexcept = 0;
  virtual int expectedSize() const noexcept { return 0; }
  virtual int pushArguments(lua_State *) const { return 0; }
  virtual void collectReturned(lua_State *) {}

  virtual bool findCallback(lua_State *L) const;
};

class DiscardCallback : public PluginCallback
{
public:
  inline constexpr int expectedSize() const noexcept override { return 1; }
  constexpr DiscardCallback() : processing(true) {}
  void collectReturned(lua_State *L) override;
  inline constexpr bool discarded() const { return !processing; }

private:
  bool processing;
};

class ModifyTextCallback : public PluginCallback
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

class OnPluginBroadcast : public PluginCallback
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
  CALLBACK(1, "OnPluginCommand", ActionSource::UserTyping)
  constexpr OnPluginCommand(const QByteArray &text) : DiscardCallback(), text(text) {}
  int pushArguments(lua_State *L) const override;

private:
  const QByteArray &text;
};

class OnPluginCommandChanged : public PluginCallback
{
public:
  CALLBACK(2, "OnPluginCommandChanged", ActionSource::UserTyping)
};

class OnPluginClose : public PluginCallback
{
public:
  CALLBACK(3, "OnPluginClose", ActionSource::Unknown)
};

class OnPluginCommandEntered : public ModifyTextCallback
{
public:
  CALLBACK(4, "OnPluginCommandEntered", ActionSource::UserTyping)
  constexpr OnPluginCommandEntered(QByteArray &text) : ModifyTextCallback(text) {}
};

class OnPluginConnect : public PluginCallback
{
public:
  CALLBACK(5, "OnPluginConnect", ActionSource::WorldAction)
};

class OnPluginDisconnect : public PluginCallback
{
public:
  CALLBACK(6, "OnPluginDisconnect", ActionSource::WorldAction)
};

class OnPluginGetFocus : public PluginCallback
{
public:
  CALLBACK(7, "OnPluginGetFocus", ActionSource::WorldAction)
};

class OnPluginIacGa : public PluginCallback
{
public:
  CALLBACK(8, "OnPlugin_IAC_GA", ActionSource::Unknown)
};

class OnPluginInstall : public PluginCallback
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

class OnPluginListChanged : public PluginCallback
{
public:
  CALLBACK(11, "OnPluginListChanged", ActionSource::Unknown)
};

class OnPluginLoseFocus : public PluginCallback
{
public:
  CALLBACK(12, "OnPluginLoseFocus", ActionSource::WorldAction)
};

class OnPluginMXPStart : public PluginCallback
{
public:
  CALLBACK(13, "OnPluginMXPstart", ActionSource::WorldAction)
};

class OnPluginMXPStop : public PluginCallback
{
public:
  CALLBACK(14, "OnPluginMXPstop", ActionSource::WorldAction)
};

class OnPluginMXPSetEntity : public PluginCallback
{
public:
  CALLBACK(15, "OnPluginMXPsetEntity", ActionSource::WorldAction)
  constexpr OnPluginMXPSetEntity(std::string_view value) : value(value) {}
  int pushArguments(lua_State *L) const override;

private:
  std::string_view value;
};

class OnPluginMXPSetVariable : public PluginCallback
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

class OnPluginSaveState : public PluginCallback
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

class OnPluginSent : public PluginCallback
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

class OnPluginTelnetRequest : public PluginCallback
{
public:
  CALLBACK(21, "OnPluginTelnetRequest", ActionSource::Unknown)
  constexpr OnPluginTelnetRequest(uint8_t code, std::string_view message) : code(code), message(message) {}
  int pushArguments(lua_State *L) const override;

private:
  int code;
  std::string_view message;
};

class OnPluginTelnetSubnegotiation : public PluginCallback
{
public:
  CALLBACK(22, "OnPluginTelnetSubnegotiation", ActionSource::Unknown)
  constexpr OnPluginTelnetSubnegotiation(uint8_t code, const QByteArray &data) : code(code), data(data) {}
  int pushArguments(lua_State *L) const override;

private:
  int code;
  const QByteArray &data;
};

class OnPluginWorldSave : public PluginCallback
{
public:
  CALLBACK(23, "OnPluginWorldSave", ActionSource::Unknown)
};

class OnPluginWorldOutputResized : public PluginCallback
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
  bool setIfDefined(lua_State *L, const PluginCallback &callback);

private:
  size_t filter = 0;
};

#undef CALLBACK
