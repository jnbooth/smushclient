#pragma once

enum struct ApiCode
{
  OK = 0,                             // No error
  WorldOpen = 30001,                  // The world is already open
  WorldClosed = 30002,                // The world is closed, this action cannot be performed
  NoNameSpecified = 30003,            // No name has been specified where one is required
  CannotPlaySound = 30004,            // The sound file could not be played
  TriggerNotFound = 30005,            // The specified trigger name does not exist
  TriggerAlreadyExists = 30006,       // Attempt to add a trigger that already exists
  TriggerCannotBeEmpty = 30007,       // The trigger "match" string cannot be empty
  InvalidObjectLabel = 30008,         // The name of this object is invalid
  ScriptNameNotLocated = 30009,       // Script name is not in the script file
  AliasNotFound = 30010,              // The specified alias name does not exist
  AliasAlreadyExists = 30011,         // Attempt to add a alias that already exists
  AliasCannotBeEmpty = 30012,         // The alias "match" string cannot be empty
  CouldNotOpenFile = 30013,           // Unable to open requested file
  LogFileNotOpen = 30014,             // Log file was not open
  LogFileAlreadyOpen = 30015,         // Log file was already open
  LogFileBadWrite = 30016,            // Bad write to log file
  TimerNotFound = 30017,              // The specified timer name does not exist
  TimerAlreadyExists = 30018,         // Attempt to add a timer that already exists
  VariableNotFound = 30019,           // Attempt to delete a variable that does not exist
  CommandNotEmpty = 30020,            // Attempt to use SetCommand with a non-empty command window
  BadRegularExpression = 30021,       // Bad regular expression syntax
  TimeInvalid = 30022,                // Time given to AddTimer is invalid
  BadMapItem = 30023,                 // Direction given to AddToMapper is invalid
  NoMapItems = 30024,                 // No items in mapper
  UnknownOption = 30025,              // Option name not found
  OptionOutOfRange = 30026,           // New value for option is out of range
  TriggerSequenceOutOfRange = 30027,  // Trigger sequence value invalid
  TriggerSendToInvalid = 30028,       // Where to send trigger text to is invalid
  TriggerLabelNotSpecified = 30029,   // Trigger label not specified/invalid for 'send to variable'
  PluginFileNotFound = 30030,         // File name specified for plugin not found
  ProblemsLoadingPlugin = 30031,      // There was a parsing or other problem loading the plugin
  PluginCannotSetOption = 30032,      // Plugin is not allowed to set this option
  PluginCannotGetOption = 30033,      // Plugin is not allowed to get this option
  NoSuchPlugin = 30034,               // Requested plugin is not installed
  NotAPlugin = 30035,                 // Only a plugin can do this
  NoSuchRoutine = 30036,              // Plugin does not support that subroutine (subroutine not in script)
  PluginDoesNotSaveState = 30037,     // Plugin does not support saving state
  PluginCouldNotSaveState = 30037,    // Plugin could not save state (eg. no state directory)
  PluginDisabled = 30039,             // Plugin is currently disabled
  ErrorCallingPluginRoutine = 30040,  // Could not call plugin routine
  CommandsNestedTooDeeply = 30041,    // Calls to "Execute" nested too deeply
  CannotCreateChatSocket = 30042,     // Unable to create socket for chat connection
  CannotLookupDomainName = 30043,     // Unable to do DNS (domain name) lookup for chat connection
  NoChatConnections = 30044,          // No chat connections open
  ChatPersonNotFound = 30045,         // Requested chat person not connected
  BadParameter = 30046,               // General problem with a parameter to a script call
  ChatAlreadyListening = 30047,       // Already listening for incoming chats
  ChatIDNotFound = 30048,             // Chat session with that ID not found
  ChatAlreadyConnected = 30049,       // Already connected to that server/port
  ClipboardEmpty = 30050,             // Cannot get (text from the) clipboard
  FileNotFound = 30051,               // Cannot open the specified file
  AlreadyTransferringFile = 30052,    // Already transferring a file
  NotTransferringFile = 30053,        // Not transferring a file
  NoSuchCommand = 30054,              // There is not a command of that name
  ArrayAlreadyExists = 30055,         // That array already exists
  BadKeyName = 30056,                 // That name is not permitted for a key
  ArrayDoesNotExist = 30056,          // That array does not exist
  ArrayNotEvenNumberOfValues = 30057, // Values to be imported into array are not in pairs
  ImportedWithDuplicates = 30058,     // Import succeeded, however some values were overwritten
  BadDelimiter = 30059,               // Import/export delimiter must be a single character, other than backslash
  SetReplacingExistingValue = 30060,  // Array element set, existing value overwritten
  KeyDoesNotExist = 30061,            // Array key does not exist
  CannotImport = 30062,               // Cannot import because cannot find unused temporary character
  ItemInUse = 30063,                  // Cannot delete trigger/alias/timer because it is executing a script
  SpellCheckNotActive = 30064,        // Spell checker is not active
  CannotAddFont = 30065,              // Cannot create requested font
  PenStyleNotValid = 30066,           // Invalid settings for pen parameter
  UnableToLoadImage = 30067,          // Bitmap image could not be loaded
  ImageNotInstalled = 30068,          // Image has not been loaded into window
  InvalidNumberOfPoints = 30069,      // Number of points supplied is incorrect
  InvalidPoint = 30070,               // Point is not numeric
  HotspotPluginChanged = 30071,       // Hotspot processing must all be in same plugin
  HotspotNotInstalled = 30072,        // Hotspot has not been defined for this window
  NoSuchWindow = 30073,               // Requested miniwindow does not exist
  BrushStyleNotValid = 30074,         // Invalid settings for brush parameter
};

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
  AliasQueue = 4096,       // Queue this alias for sending at the speedwalking delay interval
  AliasMenu = 8192,        // This alias appears on the alias menu
  Temporary = 16384,       // Temporary - do not save to world file
  OneShot = 32768,         // If set, only fires once
};
Q_DECLARE_FLAGS(AliasFlags, AliasFlag)

enum struct ActionSource
{
  Unknown,         // No particular reason, could be plugin saving
  UserTyping,      // User typed something in the command area and pressed <Enter>
  UserMacro,       // User typed a macro (e.g. F2) (unused)
  UserKeypad,      // User used the numeric keypad
  UserAccelerator, // User used a hotkey (unused)
  UserMenuAction,  // Item chosen from pop-up menu
  TriggerFired,    // Trigger fired
  TimerFired,      // Timer fired
  InputFromServer, // Input arrived (eg. packet received)
  WorldAction,     // Some sort of world action (e.g. world open, connect, got focus)
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

  // NOT IMPLEMENTED

  // Draws a 3D-style edge with optional fill.
  Edge3D,
  // Fills to border specified by the first color parameter. The filling commences at the pixel
  // designated by Left, Top and continues until it runs out of pixels that color. The fill color
  // is specified by the second color parameter.
  FloodFillBorder,
  // Fills while on surface specified by the first color pmarameter. The filling commences at the
  // pixel designated by Left, Top and continues until it is on pixels that color. The fill color
  // is specified by Colour2.
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
Q_DECLARE_FLAGS(TimerFlags, TimerFlag)

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
Q_DECLARE_FLAGS(TriggerFlags, TriggerFlag)

