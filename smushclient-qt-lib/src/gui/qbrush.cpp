#include "smushclient-qt-lib/qbrush.h"
#include <cxx-qt-lib/assertion_utils.h>

assert_alignment_and_size(QBrush, { ::std::size_t a0; });

static_assert(!::std::is_trivially_copy_assignable<QBrush>::value);
static_assert(!::std::is_trivially_copy_constructible<QBrush>::value);
static_assert(!::std::is_trivially_destructible<QBrush>::value);
static_assert(::std::is_move_constructible<QBrush>::value);
static_assert(QTypeInfo<QBrush>::isRelocatable);
