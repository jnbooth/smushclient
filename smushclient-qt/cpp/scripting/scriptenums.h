#pragma once
#include "smushclient_qt/src/ffi/api_code.cxx.h"
#include <QtCore/QFlags>

enum class AliasFlag
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
Q_DECLARE_FLAGS(AliasFlags, AliasFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(AliasFlags)

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

enum class CircleOp : int64_t
{
  Ellipse = 1,
  Rectangle,
  RoundedRectangle,
  Chord,
  Pie,
};

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

enum FontPitchFlag : int64_t
{
  Default = 0,
  Fixed = 1,
  Variable = 2,
  Monospace = 8
};

enum FontFamilyFlag : int64_t
{
  AnyFamily = 0,
  Roman = 16,
  Swiss = 32,
  Modern = 48,
  Script = 64,
  Decorative = 80,
};

enum class ImageOp : int64_t
{
  Ellipse = 1,
  Rectangle,
  RoundedRectangle,
};

enum class OperatingSystem : int64_t
{
  Windows = 2,
  MacOS = 100,
  Linux = 200,
};

enum PenStyleFlag : int64_t
{
  SolidLine = 0,
  DashLine = 1,
  DotLine = 2,
  DashDotLine = 3,
  DashDotDotLine = 4,
  NoPen = 5,
  InsideFrame = 6,
};

enum PenCapFlag : int64_t
{
  RoundCap = 0x000,
  SquareCap = 0x100,
  FlatCap = 0x200,
};

enum PenJoinFlag : int64_t
{
  RoundJoin = 0x0000,
  BevelJoin = 0x1000,
  MiterJoin = 0x2000,
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

enum class TimerFlag
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
Q_DECLARE_FLAGS(TimerFlags, TimerFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(TimerFlags)

enum class TriggerFlag
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
Q_DECLARE_FLAGS(TriggerFlags, TriggerFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(TriggerFlags)
