#include "smushclient-qt-lib/qfontinfo.h"
#include <cxx-qt-lib/assertion_utils.h>

assert_alignment_and_size(QFontInfo, { ::std::size_t a0; });

static_assert(!::std::is_trivially_copy_assignable<QFontInfo>::value);
static_assert(!::std::is_trivially_copy_constructible<QFontInfo>::value);
static_assert(!::std::is_trivially_destructible<QFontInfo>::value);
static_assert(::std::is_move_constructible<QFontInfo>::value);
static_assert(QTypeInfo<QFontInfo>::isRelocatable);
