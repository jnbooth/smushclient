#pragma once

#include <QtGui/QBrush>

#include "rust/cxx.h"

namespace rust {
template<>
struct IsRelocatable<QBrush> : ::std::true_type
{};

} // namespace rust
