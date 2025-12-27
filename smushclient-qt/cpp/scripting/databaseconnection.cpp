#include "databaseconnection.h"
#include "sqlite3.h"

using std::string;
using std::string_view;

// Private utils

inline string replacePathSeparators(string_view path) {
  string file = (string)path;
  for (auto iter = file.begin(), end = file.end(); iter != end; ++iter)
    if (*iter == '\\')
      *iter = '/';
  return file;
}

// Public methods

DatabaseConnection::DatabaseConnection(string_view filename)
    : filename(replacePathSeparators(filename)) {}

DatabaseConnection::DatabaseConnection(DatabaseConnection &&other) noexcept
    : db(std::exchange(other.db, nullptr)), filename(std::move(other.filename)),
      stmt(std::exchange(other.stmt, nullptr)), validRow(other.validRow) {}

DatabaseConnection::~DatabaseConnection() { close(); }

int DatabaseConnection::close() {
  if (stmt) {
    sqlite3_finalize(stmt);
    stmt = nullptr;
  }

  if (!db)
    return -2;

  const int result = sqlite3_close(db);
  db = nullptr;
  return result;
}

int DatabaseConnection::open(int flags) {
  return sqlite3_open_v2(filename.c_str(), &db, flags, nullptr);
}
