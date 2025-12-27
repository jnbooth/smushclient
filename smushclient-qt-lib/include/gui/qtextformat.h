#pragma once

#include <QtGui/QTextFormat>

#include "rust/cxx.h"

using QFontWeight = QFont::Weight;
using QTextFormatFormatType = QTextFormat::FormatType;
using QTextFormatObjectTypes = QTextFormat::ObjectTypes;
using QTextFormatPageBreakFlag = QTextFormat::PageBreakFlag;
using QTextFormatPageBreakFlags = QTextFormat::PageBreakFlags;
using QTextFormatProperty = QTextFormat::Property;

namespace rust {
template<>
struct IsRelocatable<QTextFormat> : ::std::true_type
{};

} // namespace rust
