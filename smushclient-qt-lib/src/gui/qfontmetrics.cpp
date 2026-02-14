#include "smushclient-qt-lib/qfontmetrics.h"
#include <cxx-qt-lib/assertion_utils.h>

assert_alignment_and_size(QFontMetrics, { ::std::size_t a0; });

static_assert(!::std::is_trivially_copy_assignable<QFontMetrics>::value);
static_assert(!::std::is_trivially_copy_constructible<QFontMetrics>::value);
static_assert(!::std::is_trivially_destructible<QFontMetrics>::value);
static_assert(::std::is_move_constructible<QFontMetrics>::value);
static_assert(QTypeInfo<QFontMetrics>::isRelocatable);
