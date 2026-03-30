#include "../../ui/worldtab.h"
#include "../callback/plugincallback.h"
#include "../scriptapi.h"
#include "sqlite3.h"

using std::string;
using std::string_view;

// Public methods

int
ScriptApi::DatabaseClose(string_view databaseID)
{
  auto search = databases.find(databaseID);
  if (search == databases.end()) [[unlikely]] {
    return DatabaseConnection::Error::IdNotFound;
  }

  const int result = search->second.close();
  databases.erase(search);
  return result;
}

int
ScriptApi::DatabaseOpen(string_view databaseID, string_view filename, int flags)
{
  auto entry = databases.emplace(string(databaseID), filename);
  DatabaseConnection& db = entry.first->second;
  if (!entry.second) {
    return db.isFile(databaseID)
             ? SQLITE_OK
             : DatabaseConnection::Error::DatabaseAlreadyExists;
  }

  const int result = db.open(flags);
  if (result != SQLITE_OK) {
    databases.erase(entry.first);
  }

  return result;
}

bool
ScriptApi::Save(const QString& path, bool replace)
{
  return !(path.isEmpty() ? tab.saveWorldAsNew(path, !replace)
                          : tab.saveWorld())
            .isEmpty();
}

ApiCode
ScriptApi::SaveState(size_t plugin)
{
  OnPluginSaveState onSaveState;
  sendCallback(onSaveState, plugin);
  try {
    client.trySaveState(plugin, tab.variablesPath());
  } catch (const rust::Error&) {
    return ApiCode::PluginCouldNotSaveState;
  }
  return ApiCode::OK;
}

void
ScriptApi::SetChanged(bool changed) const
{
  tab.setWindowModified(changed);
}
