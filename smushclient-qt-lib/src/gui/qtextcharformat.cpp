#include "smushclient-qt-lib/qtextcharformat.h"
#include <cxx-qt-lib/assertion_utils.h>

assert_alignment_and_size(QTextCharFormat, {
  ::std::size_t a0;
  ::std::int32_t format_type;
});

static_assert(!::std::is_trivially_copy_assignable<QTextCharFormat>::value);
static_assert(!::std::is_trivially_copy_constructible<QTextCharFormat>::value);
static_assert(!::std::is_trivially_destructible<QTextCharFormat>::value);
static_assert(::std::is_move_constructible<QTextCharFormat>::value);
static_assert(QTypeInfo<QTextCharFormat>::isRelocatable);

namespace rust {
namespace smushclientqtlib1 {
void qtextcharformatSetFont(QTextCharFormat &format, const QFont &font) {
  format.setFont(font);
}
void qtextcharformatSetFontWithSpecified(QTextCharFormat &format,
                                         const QFont &font) {
  format.setFont(font, QTextCharFormat::FontPropertiesSpecifiedOnly);
}
QTextCharFormat qtextformatToCharFormat(const QTextFormat &format) {
  return format.toCharFormat();
}
} // namespace smushclientqtlib1
} // namespace rust
