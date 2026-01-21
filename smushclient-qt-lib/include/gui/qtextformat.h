#pragma once

#include <QtGui/QTextFormat>

#include "rust/cxx.h"

namespace rust {
template<>
struct IsRelocatable<QTextFormat> : ::std::true_type
{};

namespace smushclientqtlib1 {
using QFontWeight = QFont::Weight;
using QTextFormatFormatType = QTextFormat::FormatType;
using QTextFormatObjectTypes = QTextFormat::ObjectTypes;
using QTextFormatPageBreakFlag = QTextFormat::PageBreakFlag;
using QTextFormatPageBreakFlags = QTextFormat::PageBreakFlags;
using QTextFormatProperty = QTextFormat::Property;
} // namespace smushclientqtlib1
} // namespace rust
