#pragma once
#include <string>

class sqlite3;
class sqlite3_stmt;

class DatabaseConnection
{
public:
  DatabaseConnection(std::string_view filename);
  DatabaseConnection(DatabaseConnection &&other);
  ~DatabaseConnection();

  DatabaseConnection(const DatabaseConnection &) = delete;
  DatabaseConnection &operator=(const DatabaseConnection &) = delete;

  int close();
  inline constexpr bool isFile(std::string_view file) const noexcept { return filename == file; }
  int open(int flags);

private:
  sqlite3 *db = nullptr;
  std::string filename;
  bool moved = false;
  sqlite3_stmt *stmt = nullptr;
  bool validRow = false;
};
