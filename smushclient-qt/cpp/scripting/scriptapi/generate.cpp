#include "../scriptapi.h"
#include "smushclient_qt/src/ffi/regex.cxx.h"
#include "smushclient_qt/src/ffi/util.cxx.h"

using std::array;
using std::string_view;

// Private utils

namespace {
rust::Slice<const uint8_t>
slice(QByteArrayView buf)
{
  return rust::Slice(reinterpret_cast<const uint8_t*>(buf.data()), buf.size());
}

template<size_t N>
rust::Slice<uint8_t>
mutSlice(array<char, N>& buf)
{
  return rust::Slice(reinterpret_cast<uint8_t*>(buf.data()), buf.size());
}
} // namespace

// Public static methods

// NOLINTBEGIN(cppcoreguidelines-pro-type-member-init, hicpp-member-init)

array<char, 36>
ScriptApi::CreateGUID()
{
  array<char, 36> buf;
  const size_t size = ffi::util::create_guid(mutSlice(buf)).size();
  Q_ASSERT(size == buf.size());
  return buf;
}

array<char, 24>
ScriptApi::GetUniqueID()
{
  array<char, 24> buf;
  const size_t size = ffi::util::get_unique_id(mutSlice(buf)).size();
  Q_ASSERT(size == buf.size());
  return buf;
}

int64_t
ScriptApi::GetUniqueNumber() noexcept
{
  static int64_t uniqueNumber = -1;
  if (uniqueNumber == INT64_MAX) [[unlikely]] {
    uniqueNumber = 0;
  } else {
    ++uniqueNumber;
  }
  return uniqueNumber;
}

array<char, 40>
ScriptApi::Hash(QByteArrayView bytes)
{
  array<char, 40> buf;
  const size_t size = ffi::util::hash(slice(bytes), mutSlice(buf)).size();
  Q_ASSERT(size == buf.size());
  return buf;
}

QString
ScriptApi::MakeRegularExpression(string_view pattern) noexcept
{
  return ffi::regex::from_wildcards(pattern);
}

// NOLINTEND(cppcoreguidelines-pro-type-member-init, hicpp-member-init)

// Public methods
