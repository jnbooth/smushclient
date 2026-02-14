#pragma once

#include <QtGui/QFontMetrics>

#include "rust/cxx.h"

namespace rust {
template<>
struct IsRelocatable<QFontMetrics> : ::std::true_type
{};

} // namespace rust
