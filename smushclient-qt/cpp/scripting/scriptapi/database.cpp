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
