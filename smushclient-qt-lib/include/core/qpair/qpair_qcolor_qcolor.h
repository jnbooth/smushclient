#pragma once
#include "rust/cxx.h"
#include <QtGui/QColor>

using QPair_QColor_QColor = ::std::pair<::QColor, ::QColor>;

namespace rust {

// This has static asserts in the cpp file to ensure this is valid.
template<>
struct IsRelocatable<QPair<::QColor, ::QColor>> : ::std::true_type
{};

}
