#include "smushclient-qt-lib/qfontmetricsf.h"
#include <cxx-qt-lib/assertion_utils.h>

assert_alignment_and_size(QFontMetricsF, { ::std::size_t a0; });

static_assert(!::std::is_trivially_copy_assignable<QFontMetricsF>::value);
static_assert(!::std::is_trivially_copy_constructible<QFontMetricsF>::value);
static_assert(!::std::is_trivially_destructible<QFontMetricsF>::value);
static_assert(::std::is_move_constructible<QFontMetricsF>::value);
static_assert(QTypeInfo<QFontMetricsF>::isRelocatable);
