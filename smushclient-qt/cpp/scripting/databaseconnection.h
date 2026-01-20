#pragma once
#include <string>

struct sqlite3;
struct sqlite3_stmt;

class DatabaseConnection
{
public:
  explicit DatabaseConnection(std::string_view filename);
  DatabaseConnection(DatabaseConnection&& other) noexcept;
  ~DatabaseConnection();

  DatabaseConnection(const DatabaseConnection&) = delete;
  DatabaseConnection& operator=(const DatabaseConnection&) = delete;
  DatabaseConnection& operator=(DatabaseConnection&&) = delete;

  int close();
  constexpr bool isFile(std::string_view file) const noexcept
  {
    return filename == file;
  }
  int open(int flags);

private:
  sqlite3* db = nullptr;
  std::string filename;
  sqlite3_stmt* stmt = nullptr;
  bool validRow = false;
};
