#include "scriptenums.h"
#include <QtGui/QFont>
#include <QtGui/QFontInfo>

// Private utils

namespace {
inline ScriptFont::Pitch
getFontPitch(const QFont& font)
{
  return QFontInfo(font).fixedPitch() ? ScriptFont::Pitch::Fixed
                                      : ScriptFont::Pitch::Variable;
}

constexpr ScriptFont::Family
getFontFamily(QFont::StyleHint hint) noexcept
{
  using StyleHint = QFont::StyleHint;
  using Family = ScriptFont::Family;

  switch (hint) {
    case StyleHint::Serif:
      return Family::Roman;
    case StyleHint::SansSerif:
      return Family::Swiss;
    case StyleHint::TypeWriter:
      return Family::Modern;
    case StyleHint::Cursive:
      return Family::Script;
    case StyleHint::Decorative:
      return Family::Decorative;
    default:
      return Family::AnyFamily;
  }
}

inline int64_t
getScriptFont(const QFont& font)
{
  const QFont::StyleHint hint = font.styleHint();
  if (hint == QFont::StyleHint::Monospace) {
    return static_cast<int64_t>(ScriptFont::Pitch::Monospace);
  }

  return static_cast<int64_t>(getFontPitch(font)) |
         static_cast<int64_t>(getFontFamily(hint));
}

constexpr Qt::PenCapStyle
getPenCap(ScriptPen::Cap cap) noexcept
{
  using Cap = ScriptPen::Cap;
  using PenCapStyle = Qt::PenCapStyle;

  switch (cap) {
    case Cap::RoundCap:
      return PenCapStyle::RoundCap;
    case Cap::SquareCap:
      return PenCapStyle::SquareCap;
    case Cap::FlatCap:
      return PenCapStyle::FlatCap;
  }
}

constexpr Qt::PenJoinStyle
getPenJoin(ScriptPen::Join join) noexcept
{
  using Join = ScriptPen::Join;
  using PenJoinStyle = Qt::PenJoinStyle;

  switch (join) {
    case Join::RoundJoin:
      return PenJoinStyle::RoundJoin;
    case Join::BevelJoin:
      return PenJoinStyle::BevelJoin;
    case Join::MiterJoin:
      return PenJoinStyle::MiterJoin;
  }
}

constexpr Qt::PenStyle
getPenStyle(ScriptPen::Style style) noexcept
{
  using Style = ScriptPen::Style;
  using PenStyle = Qt::PenStyle;

  switch (style) {
    case Style::SolidLine:
      return PenStyle::SolidLine;
    case Style::DashLine:
      return PenStyle::DashLine;
    case Style::DotLine:
      return PenStyle::DotLine;
    case Style::DashDotLine:
      return PenStyle::DashDotLine;
    case Style::DashDotDotLine:
      return PenStyle::DashDotDotLine;
    case Style::NoPen:
      return PenStyle::NoPen;
    case Style::InsideFrame:
      return PenStyle::SolidLine;
  }
}

} // namespace

// Public methods

ScriptFont::ScriptFont(const QFont& font)
  : value(getScriptFont(font))
{
}

QFont::StyleHint
ScriptFont::hint() const noexcept
{
  using StyleHint = QFont::StyleHint;

  if (pitch() == Pitch::Monospace) {
    return StyleHint::Monospace;
  }

  switch (family()) {
    case Family::Roman:
      return StyleHint::Serif;
    case Family::Swiss:
      return StyleHint::SansSerif;
    case Family::Modern:
      return StyleHint::TypeWriter;
    case Family::Script:
      return StyleHint::Cursive;
    case Family::Decorative:
      return StyleHint::Decorative;
    case Family::AnyFamily:
      return StyleHint::AnyStyle;
  }
}

QPen
ScriptPen::qPen(const QBrush& brush, qreal width) const
{
  return QPen(
    brush, width, getPenStyle(style()), getPenCap(cap()), getPenJoin(join()));
}
