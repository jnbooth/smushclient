#pragma once

#include <QtGui/QTextCharFormat>

namespace rust {
namespace smushclientqtlib1 {
using QTextCharFormatUnderlineStyle = QTextCharFormat::UnderlineStyle;
using QTextCharFormatVerticalAlignment = QTextCharFormat::VerticalAlignment;

void
qtextcharformatSetFont(QTextCharFormat& format, const QFont& font);
void
qtextcharformatSetFontWithSpecified(QTextCharFormat& format, const QFont& font);
void
qtextformatToCharFormat(const QTextFormat& format, QTextCharFormat* uninit);
} // namespace smushclientqtlib1
} // namespace rust
