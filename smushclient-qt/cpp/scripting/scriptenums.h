#pragma once
#include "../enumbounds.h"
#include "smushclient_qt/src/ffi/api_code.cxx.h"
#include <QtCore/QFlags>
#include <QtGui/QFont>
#include <QtGui/QPen>

enum class ActionSource
{
  Unknown,    // No particular reason, could be plugin saving
  UserTyping, // User typed something in the command area and pressed <Enter>
  UserMacro,  // User typed a macro (e.g. F2) (unused)
  UserKeypad, // User used the numeric keypad
  UserAccelerator, // User used a hotkey (unused)
  UserMenuAction,  // Item chosen from pop-up menu
  TriggerFired,    // Trigger fired
  TimerFired,      // Timer fired
  InputFromServer, // Input arrived (eg. packet received)
  WorldAction,     // Some sort of world action (e.g. world open, connect, got
                   // focus)
  LuaSandbox,      // Executing Lua sandbox (unused)
  Hotspot,         // Hotspot callback
};
DECLARE_ENUM_BOUNDS(ActionSource, Unknown, Hotspot)

enum class BlendMode : int64_t
{
  Normal = 1,
  Average,
  Interpolate,
  Dissolve,
  Darken,
  Multiply,
  ColorBurn,
  LinearBurn,
  InverseColorBurn,
  Subtract,
  Lighten,
  Screen,
  ColorDodge,
  LinearDodge,
  InverseColorDodge,
  Add,
  Overlay,
  SoftLight,
  HardLight,
  VividLight,
  LinearLight,
  PinLight,
  HardMix,
  Difference,
  Exclusion,
  Reflect,
  Glow,
  Freeze,
  Heat,
  Negation,
  Phoenix,
  Stamp,
  Xor,
  And,
  Or,
  Red,
  Green,
  Blue,
  Yellow,
  Cyan,
  Magenta,
  GreenLimitedByRed,
  GreenLimitedByBlue,
  GreenLimitedByRedAndBlue,
  BlueLimitedByRed,
  BlueLimitedByGreen,
  BlueLimitedByRedAndGreen,
  RedLimitedByGreen,
  RedLimitedByBlue,
  RedLimitedByGreenAndBlue,
  RedOnly,
  GreenOnly,
  BlueOnly,
  DiscardRed,
  DiscardGreen,
  DiscardBlue,
  AllRed,
  AllGreen,
  AllBlue,
  Hue,
  Saturation,
  Color,
  Luminance,
  Hsl,
};
DECLARE_ENUM_BOUNDS(BlendMode, Normal, Hsl)

enum class ButtonFrame : int64_t
{
  Raised = 5,
  Etched = 6,
  Bump = 9,
  Sunken = 10,
};
template<>
struct enum_bounds<ButtonFrame>
{
  static constexpr bool validate(int64_t value) noexcept
  {
    return value == static_cast<int64_t>(ButtonFrame::Raised) ||
           value == static_cast<int64_t>(ButtonFrame::Etched) ||
           value == static_cast<int64_t>(ButtonFrame::Bump) ||
           value == static_cast<int64_t>(ButtonFrame::Sunken);
  }
};

enum class CircleOp : int64_t
{
  Ellipse = 1,
  Rectangle,
  RoundedRectangle,
  Chord,
  Pie,
};
DECLARE_ENUM_BOUNDS(CircleOp, Ellipse, Pie)

enum class DrawImageMode : int64_t
{
  // Copy without stretching to the destination position. The image is not
  // clipped, so only the Left and Top parameters are used - the full image is
  // copied to that position.
  Copy = 1,
  // Stretch or shrink the image appropriately to fit into the rectangle:
  // Left, Top, Right, Bottom.
  Stretch,
  // Copy without stretching to the position Left, Top. However this is a
  // transparent copy, where the pixel at the left,top corner (pixel position
  // 0,0) is considered the transparent colour. Any pixels that exactly match
  // that colour are not copied. WARNING - do not choose black or white as the
  // transparent colour as that throws out the calculations. Choose some other
  // colour (eg. purple) - you won't see that colour anyway.
  CopyTransparent,
};
DECLARE_ENUM_BOUNDS(DrawImageMode, Copy, CopyTransparent)

enum class FilterOp : int64_t
{
  Noise = 1,
  MonoNoise,
  Blur,
  Sharpen,
  EdgeDetect,
  Emboss,
  BrightnessAdd,
  Contrast,
  Gamma,
  RedBrightnessAdd,
  RedContrast,
  RedGamma,
  GreenBrightnessAdd,
  GreenContrast,
  GreenGamma,
  BlueBrightnessAdd,
  BlueContrast,
  BlueGamma,
  GrayscaleLinear,
  GrayscalePerceptual,
  BrightnessMult,
  RedBrightnessMult,
  GreenBrightnessMult,
  BlueBrightnessMult,
  LesserBlur,
  MinorBlur,
  Average,
};
DECLARE_ENUM_BOUNDS(FilterOp, Noise, Average)

enum class ImageOp : int64_t
{
  Ellipse = 1,
  Rectangle,
  RoundedRectangle,
};
DECLARE_ENUM_BOUNDS(ImageOp, Ellipse, RoundedRectangle)

enum class MergeMode : int64_t
{
  Straight,
  Transparent,
};
DECLARE_ENUM_BOUNDS(MergeMode, Straight, Transparent)

enum class OperatingSystem : int64_t
{
  Windows = 2,
  MacOS = 100,
  Linux = 200,
};
template<>
struct enum_bounds<OperatingSystem>
{
  static constexpr bool validate(int64_t value) noexcept
  {
    return value == static_cast<int64_t>(OperatingSystem::Windows) ||
           value == static_cast<int64_t>(OperatingSystem::MacOS) ||
           value == static_cast<int64_t>(OperatingSystem::Linux);
  }
};

enum class RectOp : int64_t
{
  Frame = 1, // Frame by a single pixel wide line
  Fill,      // Fill the entire rectangle
  Invert,    // The color on the miniwindow inside the rectangle is inverted
  Frame3D,   //  Draw a "3D-style" rectangle in two colors, a single pixel wide
  Edge3D,    // Draws a 3D-style edge with optional fill.

  // NOT IMPLEMENTED

  // Fills to border specified by the first color parameter. The filling
  // commences at the pixel designated by Left, Top and continues until it runs
  // out of pixels that color. The fill color is specified by the second color
  // parameter.
  FloodFillBorder,
  // Fills while on surface specified by the first color pmarameter. The filling
  // commences at the pixel designated by Left, Top and continues until it is on
  // pixels that color. The fill color is specified by Colour2.
  FloodFillSurface,
};
DECLARE_ENUM_BOUNDS(RectOp, Frame, FloodFillSurface)

enum class ScriptBrush : int64_t
{
  SolidPattern,
  NoBrush,
  HorPattern,
  VerPattern,
  FDiagPattern,
  BDiagPattern,
  CrossPattern,
  DiagCrossPattern,
  Dense4Pattern,
  Dense2Pattern,
  Dense1Pattern,
  HorWaves,
  VerWaves,
};
DECLARE_ENUM_BOUNDS(ScriptBrush, SolidPattern, VerWaves)

enum class ScriptCursor : int64_t
{
  BlankCursor = -1,
  ArrowCursor,
  OpenHandCursor,
  IBeamCursor,
  CrossCursor,
  WaitCursor,
  UpArrowCursor,
  SizeFDiagCursor,
  SizeBDiagCursor,
  SizeHorCursor,
  SizeVerCursor,
  SizeAllCursor,
  ForbiddenCursor,
  WhatsThisCursor,
};
DECLARE_ENUM_BOUNDS(ScriptCursor, BlankCursor, WhatsThisCursor)

enum StyleFlag : int64_t
{
  Bold = 1,
  Italic = 2,
  Underline = 4,
  Strikeout = 8,
};
Q_DECLARE_FLAGS(StyleFlags, StyleFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(StyleFlags)

enum class SysColor : int64_t
{
  Scrollbar,
  Background,
  ActiveCaption,
  InactiveCaption,
  Menu,
  Window,
  WindowFrame,
  MenuText,
  WindowText,
  CaptionText,
  ActiveBorder,
  InactiveBorder,
  ApplicationWorkspace,
  Highlight,
  HighlightText,
  ButtonFace,
  ButtonShadow,
  GrayText,
  ButtonText,
  InactiveCaptionText,
  ButtonHighlight,
  Dark,
  Light,
  InfoText,
  InfoBk,
};
DECLARE_ENUM_BOUNDS(SysColor, Scrollbar, InfoBk)

namespace senderflags {
namespace alias {
enum AliasFlag
{
  Enabled = 1,             // Enable
  KeepEvaluating = 8,      // Keep evaluating
  IgnoreAliasCase = 32,    // Ignore case when matching
  OmitFromLogFile = 64,    // Omit from log file
  RegularExpression = 128, // Uses regular expression
  ExpandVariables = 512,   // Expand variables like @direction
  Replace = 1024,          // Replace existing of same name
  AliasSpeedWalk = 2048,   // Interpret send string as a speed walk string
  AliasQueue =
    4096, // Queue this alias for sending at the speedwalking delay interval
  AliasMenu = 8192,  // This alias appears on the alias menu
  Temporary = 16384, // Temporary - do not save to world file
  OneShot = 32768,   // If set, only fires once
};
} // namespace alias

namespace timer {
enum TimerFlag
{
  Enabled = 1,           // Enable
  AtTime = 2,            // If not set, time is "every"
  OneShot = 4,           // If set, only fires once
  TimerSpeedWalk = 8,    // Timer does a speed walk when it fires
  TimerNote = 16,        // Timer does a world.Note when it fires
  ActiveWhenClosed = 32, // Timer fires even when world is disconnected
  Replace = 1024,        // Replace existing of same name
  Temporary = 16384,     // Temporary - do not save to world file
};
} // namespace timer

namespace trigger {
enum TriggerFlag
{
  Enabled = 1,              // Enable
  OmitFromLog = 2,          // Omit from log file
  OmitFromOutput = 4,       // Omit from output
  KeepEvaluating = 8,       // Keep evaluating
  IgnoreCase = 16,          // Ignore case when matching
  RegularExpression = 32,   // Uses regular expression
  ExpandVariables = 512,    // Expand variables like @direction
  Replace = 1024,           // Replace existing of same name
  LowercaseWildcard = 2048, // Wildcards forced to lower-case
  Temporary = 16384,        // Temporary - do not save to world file
  OneShot = 32768,          // If set, only fires once
};
} // namespace trigger
} // namespace senderflags

using AliasFlag = senderflags::alias::AliasFlag;
Q_DECLARE_FLAGS(AliasFlags, AliasFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(AliasFlags)

using TimerFlag = senderflags::timer::TimerFlag;
Q_DECLARE_FLAGS(TimerFlags, TimerFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(TimerFlags)

using TriggerFlag = senderflags::trigger::TriggerFlag;
Q_DECLARE_FLAGS(TriggerFlags, TriggerFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(TriggerFlags)

class ScriptFont
{
public:
  enum class Family : int64_t
  {
    AnyFamily = 0,
    Roman = 16,
    Swiss = 32,
    Modern = 48,
    Script = 64,
    Decorative = 80,
  };

  enum class Pitch : int64_t
  {
    Default = 0,
    Fixed = 1,
    Variable = 2,
    Monospace = 8
  };

public:
  static constexpr std::optional<ScriptFont> validate(int64_t value) noexcept
  {
    const ScriptFont font(value);
    return font.isValid() ? std::optional(font) : std::nullopt;
  }

  constexpr ScriptFont() noexcept = default;

  explicit constexpr ScriptFont(int64_t value) noexcept
    : value(value)
  {
  }

  explicit constexpr ScriptFont(Family family,
                                Pitch pitch = Pitch::Default) noexcept
    : value(static_cast<int64_t>(family) | static_cast<int64_t>(pitch))
  {
  }

  explicit constexpr ScriptFont(Pitch pitch) noexcept
    : value(static_cast<int64_t>(pitch))
  {
  }

  explicit ScriptFont(const QFont& font);

  explicit constexpr operator int64_t() const noexcept { return value; }

  constexpr Family family() const noexcept
  {
    return static_cast<Family>(value & ~0xF);
  }

  constexpr Pitch pitch() const noexcept
  {
    return static_cast<Pitch>(value & 0xB); // exclude 4-bit
  }

  constexpr bool isValid() const noexcept
  {
    return value == 0 || (value > 0 && value <= 92 &&
                          /// 1, 2, or 8
                          std::popcount(static_cast<uint16_t>(pitch())) == 1);
  }

  QFont::StyleHint hint() const noexcept;

private:
  int64_t value = 0;
};

class ScriptPen
{
public:
  enum class Cap : int64_t
  {
    RoundCap = 0x000,
    SquareCap = 0x100,
    FlatCap = 0x200,
  };

  enum class Join : int64_t
  {
    RoundJoin = 0x0000,
    BevelJoin = 0x1000,
    MiterJoin = 0x2000,
  };

  enum class Style : int64_t
  {
    SolidLine = 0,
    DashLine = 1,
    DotLine = 2,
    DashDotLine = 3,
    DashDotDotLine = 4,
    NoPen = 5,
    InsideFrame = 6,
  };

public:
  static constexpr std::optional<ScriptPen> validate(int64_t value) noexcept
  {
    const ScriptPen pen(value);
    return pen.isValid() ? std::optional(pen) : std::nullopt;
  }

  constexpr ScriptPen() noexcept = default;

  explicit constexpr ScriptPen(int64_t value) noexcept
    : value(value)
  {
  }

  constexpr ScriptPen(Join join, Cap cap, Style style) noexcept
    : value(static_cast<int64_t>(join) | static_cast<int64_t>(cap) |
            static_cast<int64_t>(style))
  {
  }

  explicit constexpr operator int64_t() const noexcept { return value; }

  constexpr Cap cap() const noexcept { return static_cast<Cap>(value & 0xF00); }

  constexpr Join join() const noexcept
  {
    return static_cast<Join>(value & ~0XFFF);
  }
  constexpr Style style() const noexcept
  {
    return static_cast<Style>(value & 0xFF);
  }

  constexpr bool isValid() const noexcept
  {
    return value == 0 ||
           (value > 0 && join() <= Join::MiterJoin && cap() <= Cap::FlatCap &&
            style() <= Style::InsideFrame);
  }

  QPen qPen(const QBrush& brush, qreal width = 1) const;

private:
  int64_t value = 0;
};
