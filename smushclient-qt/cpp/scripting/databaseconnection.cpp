#include "databaseconnection.h"
#include "sqlite3.h"

using std::string_view;

DatabaseConnection::DatabaseConnection(string_view filename)
    : db(nullptr),
      filename(filename),
      stmt(nullptr),
      validRow(false) {}

DatabaseConnection::DatabaseConnection(DatabaseConnection &&other)
    : db(other.db),
      filename(std::move(other.filename)),
      stmt(other.stmt),
      validRow(other.validRow) {}

DatabaseConnection::~DatabaseConnection()
{
  close();
}

// Public methods

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
