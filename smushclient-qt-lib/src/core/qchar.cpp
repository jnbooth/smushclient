#include "smushclient-qt-lib/qbrush.h"
#include <cxx-qt-lib/assertion_utils.h>

assert_alignment_and_size(QChar, { ::std::uint16_t ucs; });

static_assert(::std::is_trivially_copy_assignable<QChar>::value);
static_assert(::std::is_trivially_copy_constructible<QChar>::value);
static_assert(::std::is_trivially_destructible<QChar>::value);
static_assert(::std::is_move_constructible<QChar>::value);
static_assert(QTypeInfo<QChar>::isRelocatable);
