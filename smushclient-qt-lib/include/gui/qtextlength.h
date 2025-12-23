#pragma once

#include <QtGui/QTextLength>

#include "rust/cxx.h"

using QTextLengthType = QTextLength::Type;

namespace rust {
template <> struct IsRelocatable<QTextLength> : ::std::true_type {};

} // namespace rust
