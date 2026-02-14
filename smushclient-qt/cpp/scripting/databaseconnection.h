#pragma once
#include "sqlite3.h"
#include <string>

struct sqlite3;
struct sqlite3_stmt;

class DatabaseConnection
{
public:
  explicit DatabaseConnection(std::string_view filename);

  int close();
  constexpr bool isFile(std::string_view file) const noexcept
  {
    return filename == file;
  }
  int open(int flags);

private:
  std::unique_ptr<sqlite3, int (*)(sqlite3*)> db;
  std::string filename;
  std::unique_ptr<sqlite3_stmt, int (*)(sqlite3_stmt*)> stmt;
};
