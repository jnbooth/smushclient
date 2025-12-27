#pragma once

#include <QtGui/QTextCharFormat>

#include "rust/cxx.h"

using QTextCharFormatUnderlineStyle = QTextCharFormat::UnderlineStyle;
using QTextCharFormatVerticalAlignment = QTextCharFormat::VerticalAlignment;
using QFontWeight = QFont::Weight;

namespace rust {
template<>
struct IsRelocatable<QTextCharFormat> : ::std::true_type
{};

namespace smushclientqtlib1 {
void
qtextcharformatSetFont(QTextCharFormat& format, const QFont& font);
void
qtextcharformatSetFontWithSpecified(QTextCharFormat& format, const QFont& font);
QTextCharFormat
qtextformatToCharFormat(const QTextFormat& format);
} // namespace smushclientqtlib1
} // namespace rust
