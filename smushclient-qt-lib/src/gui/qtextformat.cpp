#include "smushclient-qt-lib/qtextformat.h"
#include <cxx-qt-lib/assertion_utils.h>

assert_alignment_and_size(QTextFormat, {
  ::std::size_t a0;
  ::std::int32_t format_type;
});
static_assert(!::std::is_trivially_copy_assignable<QTextFormat>::value);
static_assert(!::std::is_trivially_copy_constructible<QTextFormat>::value);
static_assert(!::std::is_trivially_destructible<QTextFormat>::value);
static_assert(::std::is_move_constructible<QTextFormat>::value);
static_assert(QTypeInfo<QTextFormat>::isRelocatable);
