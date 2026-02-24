#include "../../client.h"
#include "../scriptapi.h"

using std::string_view;

ApiCode
ScriptApi::CloseLog() const
{
  return client.closeLog();
}

ApiCode
ScriptApi::FlushLog() const
{
  return client.flushLog();
}

bool
ScriptApi::IsLogOpen() const noexcept
{
  return client.isLogOpen();
}

ApiCode
ScriptApi::OpenLog(string_view logFileName, bool append) const
{
  return client.openLog(logFileName, append);
}

ApiCode
ScriptApi::WriteLog(string_view message) const
{
  return client.writeToLog(message);
}
