#pragma once

#include <QtCore/QString>
#include <sstream>

class argstream : public std::ostringstream
{
public:
  using std::ostringstream::basic_ostringstream;

  template<typename T>
  argstream& operator<<(T data)
  {
    std::ostringstream::operator<<(data);
    return *this;
  }

  template<>
  argstream& operator<<(bool data)
  {
    return data ? *this << "true" : *this << "false";
  }

  template<>
  argstream& operator<<(char data)
  {
    put(data);
    return *this;
  }

  // Must use one of the below overloads instead.
  template<>
  argstream& operator<<(const char* data) = delete;

  // NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays,
  // cppcoreguidelines-pro-bounds-array-to-pointer-decay,
  // hicpp-avoid-c-arrays)
  template<std::streamsize Size>
  argstream& operator<<(const char (&data)[Size])
  {
    write(data, Size && data[Size - 1] == 0 ? Size - 1 : Size);
    return *this;
  }
  // NOLINTEND(cppcoreguidelines-avoid-c-arrays,
  // cppcoreguidelines-pro-bounds-array-to-pointer-decay,
  // hicpp-avoid-c-arrays)

  template<>
  argstream& operator<<(std::string_view data)
  {
    write(data.data(), static_cast<std::streamsize>(data.size()));
    return *this;
  }
};
