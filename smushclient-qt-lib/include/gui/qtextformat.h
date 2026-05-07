#pragma once

#include <QtGui/QTextFormat>

namespace rust {
namespace smushclientqtlib1 {
using QFontWeight = QFont::Weight;
using QTextFormatFormatType = QTextFormat::FormatType;
using QTextFormatObjectTypes = QTextFormat::ObjectTypes;
using QTextFormatPageBreakFlag = QTextFormat::PageBreakFlag;
using QTextFormatPageBreakFlags = QTextFormat::PageBreakFlags;
using QTextFormatProperty = QTextFormat::Property;

void
qtextformatBrushProperty(const QTextFormat& format,
                         int propertyId,
                         QBrush* uninit);
} // namespace smushclientqtlib1
} // namespace rust
