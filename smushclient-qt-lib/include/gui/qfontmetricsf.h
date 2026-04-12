#pragma once

#include <QtGui/QFontMetricsF>

#include "rust/cxx.h"

namespace rust {
template<>
struct IsRelocatable<QFontMetricsF> : ::std::true_type
{};

} // namespace rust
