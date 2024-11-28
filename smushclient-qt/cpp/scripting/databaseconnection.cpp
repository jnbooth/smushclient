#include "databaseconnection.h"
#include "sqlite3.h"

using std::string;
using std::string_view;

// Private utils

inline string replacePathSeparators(string_view path)
{
  string file = (string)path;
  for (auto iter = file.begin(), end = file.end(); iter != end; ++iter)
    if (*iter == '\\')
      *iter = '/';
  return file;
}

// Public methods

DatabaseConnection::DatabaseConnection(string_view filename)
    : filename(replacePathSeparators(filename)) {}

DatabaseConnection::DatabaseConnection(DatabaseConnection &&other)
    : db(other.db),
      filename(std::move(other.filename)),
      stmt(other.stmt),
      validRow(other.validRow)
{
  other.moved = true;
}

DatabaseConnection::~DatabaseConnection()
{
  if (moved)
    return;
  close();
}

int DatabaseConnection::close()
{
  if (stmt)
  {
    sqlite3_finalize(stmt);
    stmt = nullptr;
  }

  if (!db)
    return -2;

  const int result = sqlite3_close(db);
  db = nullptr;
  return result;
}

int DatabaseConnection::open(int flags)
{
  return sqlite3_open_v2(filename.data(), &db, flags, nullptr);
}
