#pragma once
#include <string>
#include <QtCore/QByteArray>

struct lua_State;

// Abstract

class PluginCallback
{
public:
  virtual ~PluginCallback() {};
  virtual const char *name() const = 0;
  virtual int expectedSize() const { return 0; }
  virtual int pushArguments(lua_State *) const { return 0; };
  virtual void collectReturned(lua_State *) {};
};

class DiscardCallback : public PluginCallback
{
public:
  inline constexpr int expectedSize() const override { return 1; }
  DiscardCallback();
  void collectReturned(lua_State *L) override;
  inline constexpr bool discarded() const { return !processing; }

private:
  bool processing;
};

class ModifyTextCallback : public PluginCallback
{
public:
  inline constexpr int expectedSize() const override { return 1; }
  ModifyTextCallback(QByteArray &text);
  void collectReturned(lua_State *L) override;
  int pushArguments(lua_State *L) const override;

private:
  QByteArray &text;
};

// Concrete

class OnPluginBroadcast : public PluginCallback
{
public:
  OnPluginBroadcast(int message, std::string_view pluginID, std::string_view pluginName, std::string_view text);
  inline constexpr const char *name() const override { return "OnPluginBroadcast"; }
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
  inline constexpr const char *name() const override { return "OnPluginCommand"; }
  OnPluginCommand(const QByteArray &text);
  int pushArguments(lua_State *L) const override;

private:
  const QByteArray &text;
};

class OnPluginCommandChanged : public PluginCallback
{
public:
  inline constexpr const char *name() const override { return "OnPluginCommandChanged"; }
};

class OnPluginClose : public PluginCallback
{
public:
  inline constexpr const char *name() const override { return "OnPluginClose"; }
};

class OnPluginCommandEntered : public ModifyTextCallback
{
public:
  inline constexpr const char *name() const override { return "OnPluginCommandEntered"; }
  OnPluginCommandEntered(QByteArray &text);
};

class OnPluginConnect : public PluginCallback
{
public:
  inline constexpr const char *name() const override { return "OnPluginConnect"; }
};

class OnPluginDisconnect : public PluginCallback
{
public:
  inline constexpr const char *name() const override { return "OnPluginDisconnect"; }
};

class OnPluginGetFocus : public PluginCallback
{
public:
  inline constexpr const char *name() const override { return "OnPluginGetFocus"; }
};

class OnPluginIacGa : public PluginCallback
{
public:
  inline constexpr const char *name() const override { return "OnPlugin_IAC_GA"; }
};

class OnPluginInstall : public PluginCallback
{
public:
  inline constexpr const char *name() const override { return "OnPluginInstall"; }
};

class OnPluginLineReceived : public DiscardCallback
{
public:
  inline constexpr const char *name() const override { return "OnPluginLineReceived"; }
  OnPluginLineReceived(std::string_view line);
  int pushArguments(lua_State *L) const override;

private:
  std::string_view line;
};

class OnPluginListChanged : public PluginCallback
{
public:
  inline constexpr const char *name() const override { return "OnPluginListChanged"; }
};

class OnPluginLoseFocus : public PluginCallback
{
public:
  inline constexpr const char *name() const override { return "OnPluginLoseFocus"; }
};

class OnPluginSaveState : public PluginCallback
{
public:
  inline constexpr const char *name() const override { return "OnPluginSaveState"; }
};

class OnPluginSend : public DiscardCallback
{
public:
  inline constexpr const char *name() const override { return "OnPluginSend"; }
  OnPluginSend(const QByteArray &text);
  int pushArguments(lua_State *L) const override;

private:
  const QByteArray &text;
};

class OnPluginSent : public PluginCallback
{
public:
  inline constexpr const char *name() const override { return "OnPluginSend"; }
  OnPluginSent(const QByteArray &text);
  int pushArguments(lua_State *L) const override;

private:
  const QByteArray &text;
};

class OnPluginTabComplete : public ModifyTextCallback
{
public:
  inline constexpr const char *name() const override { return "OnPluginTabComplete"; }
  OnPluginTabComplete(QByteArray &text);
};

class OnPluginTelnetRequest : public PluginCallback
{
public:
  inline constexpr const char *name() const override { return "OnPluginTelnetRequest"; }
  OnPluginTelnetRequest(uint8_t code, std::string_view message);
  int pushArguments(lua_State *L) const override;

private:
  int code;
  std::string_view message;
};

class OnPluginTelnetSubnegotiation : public PluginCallback
{
public:
  inline constexpr const char *name() const override { return "OnPluginTelnetSubnegotiation"; }
  OnPluginTelnetSubnegotiation(uint8_t code, const QByteArray &data);
  int pushArguments(lua_State *L) const override;

private:
  int code;
  const QByteArray &data;
};

class OnPluginWorldSave : public PluginCallback
{
public:
  inline constexpr const char *name() const override { return "OnPluginWorldSave"; }
};


class OnPluginWorldOutputResized : public PluginCallback
{
public:
  inline constexpr const char *name() const override { return "OnPluginWorldOutputResized"; }
};
