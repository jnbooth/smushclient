#pragma once
#include "smushclient_qt/src/ffi/api_code.cxx.h"
#include <QtCore/QFlags>

enum struct AliasFlag
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
typedef QFlags<AliasFlag> AliasFlags;

enum struct ActionSource
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

enum struct CircleOp
{
  Ellipse = 1,
  Rectangle,
  RoundedRectangle,
  Chord,
  Pie,
};

enum FontPitchFlag
{
  Default = 0,
  Fixed = 1,
  Variable = 2,
  Monospace = 8
};

enum FontFamilyFlag
{
  AnyFamily = 0,
  Roman = 16,
  Swiss = 32,
  Modern = 48,
  Script = 64,
  Decorative = 80,
};

enum struct ImageOp
{
  Ellipse = 1,
  Rectangle,
  RoundedRectangle,
};

enum struct OperatingSystem
{
  Windows = 2,
  MacOS = 100,
  Linux = 200,
};

enum PenStyleFlag
{
  SolidLine = 0,
  DashLine = 1,
  DotLine = 2,
  DashDotLine = 3,
  DashDotDotLine = 4,
  NoPen = 5,
  InsideFrame = 6,
};

enum PenCapFlag
{
  RoundCap = 0x000,
  SquareCap = 0x100,
  FlatCap = 0x200,
};

enum PenJoinFlag
{
  RoundJoin = 0x0000,
  BevelJoin = 0x1000,
  MiterJoin = 0x2000,
};

enum RectOp
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

enum struct ScriptBrush
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

enum struct ScriptCursor
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

enum struct TimerFlag
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
typedef QFlags<TimerFlag> TimerFlags;

enum struct TriggerFlag
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
typedef QFlags<TriggerFlag> TriggerFlags;
