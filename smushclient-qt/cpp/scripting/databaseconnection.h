#pragma once
#include "sqlite3.h"
#include <string>

struct sqlite3;
struct sqlite3_stmt;

class DatabaseConnection
{
public:
  enum Error
  {
    // unknown database ID
    IdNotFound = -1,
    // not opened
    NotOpen = -2,
    // already have a prepared statement
    HavePreparedStatement = -3,
    // no prepared statement yet
    NoPreparedStatement = -4,
    // have not stepped to valid row
    NoValidRow = -5,
    // already have a database of that disk name
    DatabaseAlreadyExists = -6,
    // requested column out of range
    ColumnOutOfRange = -7,
  };

  explicit DatabaseConnection(std::string_view filename);

  int close();
  bool isFile(std::string_view file) const noexcept { return filename == file; }
  bool isOpen() const noexcept { return dbPtr != nullptr; }
  int open(int flags);

private:
  std::unique_ptr<sqlite3, int (*)(sqlite3*)> dbPtr;
  std::string filename;
};
