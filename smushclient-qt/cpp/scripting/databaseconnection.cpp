#include "databaseconnection.h"
#include "sqlite3.h"

using std::string;
using std::string_view;

// Private utils

namespace {
inline string
replacePathSeparators(string_view path)
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
  : db(nullptr, sqlite3_close_v2)
  , filename(replacePathSeparators(filename))
{
}

int
DatabaseConnection::close()
{
  sqlite3* dbptr = db.release();

  if (dbptr == nullptr) {
    return Error::NotOpen;
  }

  return sqlite3_close_v2(dbptr);
}

int
DatabaseConnection::open(int flags)
{
  sqlite3* dbptr = nullptr;
  const int result = sqlite3_open_v2(filename.c_str(), &dbptr, flags, nullptr);
  db.reset(dbptr);
  return result;
}
