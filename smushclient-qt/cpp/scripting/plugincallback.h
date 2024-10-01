#pragma once
#include <string>
#include <QtCore/QByteArray>

struct lua_State;

#define CALLBACK(idNumber, nameString)                                               \
  inline constexpr const char *name() const noexcept override { return nameString; } \
  inline constexpr int id() const noexcept override { return 1 << idNumber; }

// Abstract

class PluginCallback
{
public:
  virtual const char *name() const noexcept = 0;
  virtual int id() const noexcept = 0;
  virtual int expectedSize() const noexcept { return 0; }
  virtual int pushArguments(lua_State *) const { return 0; }
  virtual void collectReturned(lua_State *) {};
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
  CALLBACK(0, "OnPluginBroadcast")
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
  CALLBACK(1, "OnPluginCommand")
  constexpr OnPluginCommand(const QByteArray &text) : DiscardCallback(), text(text) {}
  int pushArguments(lua_State *L) const override;

private:
  const QByteArray &text;
};

class OnPluginCommandChanged : public PluginCallback
{
public:
  CALLBACK(2, "OnPluginCommandChanged")
};

class OnPluginClose : public PluginCallback
{
public:
  CALLBACK(3, "OnPluginCode")
};

class OnPluginCommandEntered : public ModifyTextCallback
{
public:
  CALLBACK(4, "OnPluginCommandEntered")
  constexpr OnPluginCommandEntered(QByteArray &text) : ModifyTextCallback(text) {}
};

class OnPluginConnect : public PluginCallback
{
public:
  CALLBACK(5, "OnPluginConnect")
};

class OnPluginDisconnect : public PluginCallback
{
public:
  CALLBACK(6, "OnPluginDisconnect")
};

class OnPluginGetFocus : public PluginCallback
{
public:
  CALLBACK(7, "OnPluginGetFocus")
};

class OnPluginIacGa : public PluginCallback
{
public:
  CALLBACK(8, "OnPlugin_IAC_GA")
};

class OnPluginInstall : public PluginCallback
{
public:
  CALLBACK(9, "OnPluginInstall")
};

class OnPluginLineReceived : public DiscardCallback
{
public:
  CALLBACK(10, "OnPluginLineReceived")
  constexpr OnPluginLineReceived(std::string_view line) : DiscardCallback(), line(line) {}
  int pushArguments(lua_State *L) const override;

private:
  std::string_view line;
};

class OnPluginListChanged : public PluginCallback
{
public:
  CALLBACK(11, "OnPluginListChanged")
};

class OnPluginLoseFocus : public PluginCallback
{
public:
  CALLBACK(12, "OnPluginLoseFocus")
};

class OnPluginMXPStart : public PluginCallback
{
public:
  CALLBACK(13, "OnPluginMXPstart")
};

class OnPluginMXPStop : public PluginCallback
{
public:
  CALLBACK(14, "OnPluginMXPstop")
};

class OnPluginMXPSetEntity : public PluginCallback
{
public:
  CALLBACK(15, "OnPluginMXPsetEntity")
  constexpr OnPluginMXPSetEntity(std::string_view value) : value(value) {}
  int pushArguments(lua_State *L) const override;

private:
  std::string_view value;
};

class OnPluginMXPSetVariable : public PluginCallback
{
public:
  CALLBACK(16, "OnPluginMXPsetVariable")
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
  CALLBACK(17, "OnPluginSaveState")
};

class OnPluginSend : public DiscardCallback
{
public:
  CALLBACK(18, "OnPluginSend")
  constexpr OnPluginSend(const QByteArray &text) : DiscardCallback(), text(text) {}
  int pushArguments(lua_State *L) const override;

private:
  const QByteArray &text;
};

class OnPluginSent : public PluginCallback
{
public:
  CALLBACK(19, "OnPluginSend")
  constexpr OnPluginSent(const QByteArray &text) : text(text) {}
  int pushArguments(lua_State *L) const override;

private:
  const QByteArray &text;
};

class OnPluginTabComplete : public ModifyTextCallback
{
public:
  CALLBACK(20, "OnPluginTabComplete")
  constexpr OnPluginTabComplete(QByteArray &text) : ModifyTextCallback(text) {}
};

class OnPluginTelnetRequest : public PluginCallback
{
public:
  CALLBACK(21, "OnPluginTelnetRequest")
  constexpr OnPluginTelnetRequest(uint8_t code, std::string_view message) : code(code), message(message) {}
  int pushArguments(lua_State *L) const override;

private:
  int code;
  std::string_view message;
};

class OnPluginTelnetSubnegotiation : public PluginCallback
{
public:
  CALLBACK(22, "OnPluginTelnetSubnegotiation")
  constexpr OnPluginTelnetSubnegotiation(uint8_t code, const QByteArray &data) : code(code), data(data) {}
  int pushArguments(lua_State *L) const override;

private:
  int code;
  const QByteArray &data;
};

class OnPluginWorldSave : public PluginCallback
{
public:
  CALLBACK(23, "OnPluginWorldSave")
};

class OnPluginWorldOutputResized : public PluginCallback
{
public:
  CALLBACK(24, "OnPluginWorldOutputResized")
};

class CallbackFilter
{
public:
  CallbackFilter();
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
  size_t filter;
};

#undef CALLBACK
