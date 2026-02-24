#include "databaseconnection.h"
#include "sqlite3.h"

using std::string;
using std::string_view;

// Private utils

namespace {
inline string
replacePathSeparators(string_view path) noexcept
{
  string file(path);
  for (char& iter : file) {
    if (iter == '\\') {
      iter = '/';
    }
  }
  return file;
}
} // namespace

// Public methods

DatabaseConnection::DatabaseConnection(string_view filename)
  : dbPtr(nullptr, sqlite3_close_v2)
  , filename(replacePathSeparators(filename))
{
}

int
DatabaseConnection::close()
{
  sqlite3* db = dbPtr.release();

  if (db == nullptr) {
    return Error::NotOpen;
  }

  return sqlite3_close_v2(db);
}

int
DatabaseConnection::open(int flags)
{
  sqlite3* db = nullptr;
  const int result = sqlite3_open_v2(filename.c_str(), &db, flags, nullptr);
  dbPtr.reset(db);
  return result;
}
