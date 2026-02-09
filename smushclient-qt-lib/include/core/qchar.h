#pragma once

#include <QtCore/QChar>

#include "rust/cxx.h"

namespace rust {
template<>
struct IsRelocatable<QChar> : ::std::true_type
{};

} // namespace rust
