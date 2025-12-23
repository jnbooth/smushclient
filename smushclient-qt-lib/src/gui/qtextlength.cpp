#include "smushclient-qt-lib/qtextlength.h"
#include <cxx-qt-lib/assertion_utils.h>

assert_alignment_and_size(QTextLength, {
  ::std::int32_t lengthType;
  double fixedValueOrPercentage;
});

static_assert(::std::is_trivially_copyable<QTextLength>::value);
