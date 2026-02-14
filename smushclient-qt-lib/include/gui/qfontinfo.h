#pragma once

#include <QtGui/QFontInfo>

#include "rust/cxx.h"

namespace rust {
template<>
struct IsRelocatable<QFontInfo> : ::std::true_type
{};

} // namespace rust
