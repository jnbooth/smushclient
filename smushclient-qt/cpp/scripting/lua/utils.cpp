#include "utils.h"
#include "../../settings.h"
#include "../../ui/scripting/choose.h"
#include "../../ui/scripting/inputbox.h"
#include "../../ui/scripting/listbox.h"
#include "../callback/plugincallback.h"
#include "../qlua.h"
#include "../scriptapi.h"
#include "api.h"
#include "smushclient_qt/src/ffi/util.cxx.h"
#include <QtCore/QCryptographicHash>
#include <QtCore/QTranslator>
#include <QtGui/QFontDatabase>
#include <QtWidgets/QApplication>
#include <QtWidgets/QColorDialog>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QFontDialog>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QMessageBox>
extern "C"
{
#include "lauxlib.h"
  LUALIB_API int luaopen_base64(lua_State* L);
}

#ifndef SIZE_MAX
#include <limits>
#define SIZE_MAX = std::numeric_limits<size_t>::max();
#endif

using qlua::expectMaxArgs;
using qlua::getBool;
using qlua::getBytes;
using qlua::getInt;
using qlua::getInteger;
using qlua::getQColor;
using qlua::getQFont;
using qlua::getQString;
using qlua::getString;
using qlua::push;
using qlua::pushEntry;
using qlua::pushList;

using std::array;
using std::make_unique;
using std::pair;
using std::span;
using std::string_view;
using std::unique_ptr;

DECLARE_ENUM_BOUNDS(QFontDatabase::SystemFont,
                    GeneralFont,
                    SmallestReadableFont)

namespace {
const char* const utilsRegKey = "smushclient.utils";
} // namespace

static const pair<lua_Integer, string_view> infoTypes[] = {
  { 1, "Server name" },
  { 2, "World name" },
  { 3, "Character name" },
  { 4, "Send to world - file preamble" },
  { 5, "Send to world - file postamble" },
  { 6, "Send to world - line preamble" },
  { 7, "Send to world - line postamble" },
  { 8, "World notes" },
  { 9, "Sound on new activity" },
  { 10, "Scripts editor" },
  { 11, "Log file preamble" },
  { 12, "Log file postamble" },
  { 13, "Log file line preamble - player input" },
  { 14, "Log file line preamble - notes" },
  { 15, "Log file line preamble - MUD output" },
  { 16, "Log file line postamble - player input" },
  { 17, "Log file line postamble - notes" },
  { 18, "Log file line postamble - MUD output" },
  { 19, "Speed Walk Filler" },
  { 20, "Output window font name" },
  { 21, "Speed walk prefix" },
  { 22, "Text sent on connecting" },
  { 23, "Input font name" },
  { 24, "Paste to world - file preamble" },
  { 25, "Paste to world - file postamble" },
  { 26, "Paste to world - line preamble" },
  { 27, "Paste to world - line postamble" },
  { 28, "Scripting language" },
  { 29, "Script function for world open" },
  { 30, "Script function for world close" },
  { 31, "Script function for world connect" },
  { 32, "Script function for world disconnect" },
  { 31, "Script function for world get focus" },
  { 32, "Script function for world lose focus" },
  { 35, "Script file name" },
  { 36, "Scripting prefix" },
  { 37, "Auto-say string" },
  { 38, "Auto-say override" },
  { 39, "Tab-completion defaults" },
  { 40, "Auto-log file name" },
  { 41, "Recall window - line preamble" },
  { 42, "Terminal ID (telnet negotiation)" },
  { 43, "Mapping failure message" },
  { 44, "Script function for MXP starting up" },
  { 45, "Script function for MXP closing down" },
  { 46, "Script function for MXP error" },
  { 47, "Script function for MXP tag open" },
  { 48, "Script function for MXP tag close" },
  { 49, "Script function for MXP variable set" },
  { 50, "Sound to play for beeps" },

  // (strings - calculated at runtime)

  { 51, "Current log file name" },
  { 52, "Last \"immediate\" script expression" },
  { 53, "Current status line message" },
  { 54, "World file pathname" },
  { 55, "World title" },
  { 56, "SmushClient application path name" },
  { 57, "World files default path (directory)" },
  { 58, "Log files default path (directory)" },
  { 59, "Script files default path (directory)" },
  { 60, "Plugin files default path (directory)" },
  { 61, "World IP address" },
  { 62, "Proxy server IP address" },
  { 63, "Host name (name of this PC)" },
  { 64, "Current directory path" },
  { 65, "Script function for world save" },
  { 66, "SmushClient application directory" },
  { 67, "World file directory" },
  { 68, "Startup directory" },
  { 69, "Translation file" },
  { 70, "Locale" },
  { 71, "Fixed Pitch Font" },
  { 72, "MUSHclient version" },
  { 73, "SmushClient compilation date/time" },
  { 74, "Sounds directory" },
  { 75, "Last telnet subnegotiation string received" },
  { 76, "Special font pathname" },
  { 77, "OS Version - service pack" },
  { 78, "Foreground image filename" },
  { 79, "Background image filename" },
  { 80, "LibPNG version number" },
  { 81, "LibPNG header version string" },
  { 82, "Preferences database pathname" },
  { 83, "SQLite3 version" },
  { 84, "File browsing directory" },
  { 85, "State files path (directory)" },
  { 86, "Word under mouse on mouse menu click" },
  { 87, "Last command sent" },
  { 88, "Window title last set by SetTitle" },
  { 89, "Window title last set by SetMainTitle" },

  // (booleans - calculated at runtime)

  { 101, "No Echo" },
  { 102, "Debug incoming packets" },
  { 103, "Decompressing" },
  { 104, "MXP active" },
  { 105, "Pueblo active" },
  { 106, "World close flag" },
  { 107, "Currently-connecting" },
  { 108, "OK-to-disconnect" },
  { 109, "Trace" },
  { 110, "Script file changed" },
  { 111, "World file is modified" },
  { 112, "Automapper active" },
  { 113, "World is active" },
  { 114, "Output window paused" },
  { 115, "Localization active" },
  { 118, "Variables have changed" },
  { 119, "Script engine active" },
  { 120, "Scroll bar visible" },
  { 121, "High-resolution timer available" },
  { 122, "SQLite3 is thread-safe" },

  // (numbers (longs) - calculated at runtime)

  { 201, "Total lines received" },
  { 202, "Lines received but not yet seen (new lines)" },
  { 203, "Total lines sent" },
  { 204, "Packets received" },
  { 205, "Packets sent" },
  { 206, "Total uncompressed bytes received" },
  { 207, "Total compresssed bytes received" },
  { 208, "MCCP protocol in use (0=none, 1 or 2)" },
  { 209, "MXP error count" },
  { 210, "MXP tags received" },
  { 211, "MXP entities received" },
  { 212, "Output font height" },
  { 213, "Output font width" },
  { 214, "Input font height" },
  { 215, "Input font width" },
  { 216, "Total bytes received" },
  { 217, "Total bytes sent" },
  { 218, "Count of variables" },
  { 219, "Count of triggers" },
  { 220, "Count of timers" },
  { 221, "Count of aliases" },
  { 222, "Count of queued commands" },
  { 223, "Count of mapper items" },
  { 224, "Count of lines in output window" },
  { 225, "Count of custom MXP elements" },
  { 226, "Count of custom MXP entities" },
  { 227, "Connect phase" },
  { 228, "World IP address (as long)" },
  { 229, "Proxy server IP address (as long)" },
  { 230, "Script execution depth" },
  { 231, "Log file size" },
  { 232, "High-performance timer" }, // double
  { 233, "Trigger execution time" }, // double
  { 234, "Alias execution time" },   // double
  { 235, "Number of world windows" },
  { 236, "Command window start selection column" },
  { 237, "Command window end selection column" },
  { 238, "World window display flags" },
  { 239, "Source of current action" },
  { 240, "Average character width in output window" },
  { 241, "Font height in output window" },
  { 242, "Number of lines received with bad UTF-8 in them" },
  { 243, "Fixed Pitch Font Size" },
  { 244, "Count of triggers evaluated" },
  { 245, "Count of triggers matched" },
  { 246, "Count of aliases evaluated" },
  { 247, "Count of aliases matched" },
  { 248, "Count of timers fired" },
  { 249, "Main frame window height" },
  { 250, "Main frame window width" },
  { 251, "Main toolbar window height" },
  { 252, "Main toolbar window width" },
  { 253, "Game toolbar window height" },
  { 254, "Game toolbar window width" },
  { 255, "Activity toolbar window height" },
  { 256, "Activity toolbar window width" },
  { 257, "Info bar window height" },
  { 258, "Info bar window width" },
  { 259, "Status bar window height" },
  { 260, "Status bar window width" },
  { 261, "World window non-client height" },
  { 262, "World window non-client width" },
  { 263, "World window client height" },
  { 264, "World window client width" },
  { 265, "OS Version - major" },
  { 266, "OS Version - minor" },
  { 267, "OS Version - build number" },
  { 268, "OS Version - platform ID" },
  { 269, "Foreground image mode" },
  { 270, "Background image mode" },
  { 271, "Background colour" },
  { 272, "Text rectangle - left" },
  { 273, "Text rectangle - top" },
  { 274, "Text rectangle - right" },
  { 275, "Text rectangle - bottom" },
  { 276, "Text rectangle - border offset" },
  { 277, "Text rectangle - border width" },
  { 278, "Text rectangle - outside colour" },
  { 279, "Text rectangle - outside style" },
  { 280, "Output window client height" },
  { 281, "Output window client width" },
  { 282, "Text rectangle - border colour" },
  { 283, "Mouse position - X" },
  { 284, "Mouse position - Y" },
  { 285, "Output window available" },
  { 286, "Triggers matched this session" },
  { 287, "Aliases matched this session" },
  { 288, "Timers fired this session" },
  { 289, "Last line with IAC/GA or IAC/EOR in it" },
  { 290, "Actual text rectangle - left" },
  { 291, "Actual text rectangle - top" },
  { 292, "Actual text rectangle - right" },
  { 293, "Actual text rectangle - bottom" },
  { 294, "State of keyboard modifiers" },
  { 295, "Times output window redrawn" },
  { 296, "Output window scroll bar position" },
  { 297, "High-resolution timer frequency" },
  { 298, "SQLite3 version number" },
  { 299, "ANSI code-page number" },
  { 300, "OEM code-page number" },

  // (dates - calculated at runtime)

  { 301, "Time connected" },
  { 302, "Time log file was last flushed to disk" },
  { 303, "When script file was last modified" },
  { 304, "Time now" },
  { 305, "When client started executing" },
  { 306, "When this world was created/opened" },

  // more numbers

  { 310, "Newlines received" },
};

static const lua_Integer maxInfoType = 310;

namespace {
void
doHash(lua_State* L, QCryptographicHash::Algorithm algorithm)
{
  push(L, QCryptographicHash::hash(getBytes(L, 1), algorithm).toHex());
}

void
encodeUtf8Char(lua_State* L,
               rust::String& buf,
               int idx,
               int argIdx,
               int tableIdx)
{
  int isInt;
  const lua_Integer value = lua_tointegerx(L, idx, &isInt);
  if (!isInt) [[unlikely]] {
    const lua_Number decimalValue = lua_tonumber(L, argIdx);
    if (tableIdx == 0) {
      luaL_error(
        L,
        "Unicode code (%f) at argument #%d to 'utf8encode' has decimal places",
        decimalValue,
        argIdx);
    }
    luaL_error(L,
               "Unicode code (%f) at index [%d] of table at argument #%d "
               "to 'utf8encode' has decimal places",
               decimalValue,
               tableIdx,
               argIdx);
  }
  if (!ffi::util::encode_utf8(buf, value)) [[unlikely]] {
    if (tableIdx == 0) {
      luaL_error(
        L,
        "Unicode code (%d) at argument #%d to 'utf8encode' is out of range",
        value,
        argIdx);
    }
    luaL_error(L,
               "Unicode code (%d) at index [%d] of table at argument #%d "
               "to 'utf8encode' is out of range",
               value,
               tableIdx,
               argIdx);
  }
}

inline void
encodeUtf8Char(lua_State* L, rust::String& buf, int idx)
{
  encodeUtf8Char(L, buf, idx, idx, 0);
}

QVariant
getKey(lua_State* L, int tableIdx, int idx)
{
  switch (lua_type(L, idx)) {
    case LUA_TNUMBER:
      return lua_tonumber(L, idx);
    case LUA_TSTRING:
      return QString::fromUtf8(lua_tobytes(L, idx));
    default:
      return luaL_argerror(L, tableIdx, "all keys must be strings or numbers");
  }
}

inline bool
isOptionSelected(const QVariant& value, const QVariant& selection)
{
  return value == selection;
}

inline bool
isOptionSelected(const QVariant& value, const QList<QVariant>& selection)
{
  return selection.contains(value);
}

template<typename T>
int
execScriptDialog(lua_State* L,
                 int idx,
                 AbstractScriptDialog& dialog,
                 const T& selection)
{
  luaL_argexpected(L, lua_type(L, idx) == LUA_TTABLE, idx, "table");

  for (lua_pushnil(L); lua_next(L, idx); lua_pop(L, 1)) {
    const QVariant key = getKey(L, idx, -2);
    const QString value = QString::fromUtf8(lua_tobytes(L, -1));
    dialog.addItem(value, key, isOptionSelected(key, selection));
  }

  dialog.sortItems();

  if (dialog.exec() == QDialog::Accepted) {
    return dialog.pushValue(L);
  }
  lua_pushnil(L);
  return 1;
}

constexpr string_view
formatStandardButton(QMessageBox::StandardButton button)
{
  using Button = QMessageBox::StandardButton;

  switch (button) {
    case Button::Yes:
      return "yes";
    case Button::No:
      return "no";
    case Button::Ok:
      return "ok";
    case Button::Retry:
      return "retry";
    case Button::Ignore:
      return "ignore";
    case Button::Cancel:
      return "cancel";
    case Button::Abort:
      return "abort";
    default:
      return "";
  }
}

constexpr array<QMessageBox::StandardButton, 3>
getMessageBoxButtons(string_view opt)
{
  using Button = QMessageBox::StandardButton;
  if (opt == "ok") {
    return { Button::Ok, Button::Ok, Button::Ok };
  }
  if (opt == "abortretryignore") {
    return { Button::Abort, Button::Retry, Button::Ignore };
  }
  if (opt == "okcancel") {
    return { Button::Ok, Button::Cancel, Button::Ok };
  }
  if (opt == "retrycancel") {
    return { Button::Retry, Button::Cancel, Button::Retry };
  }
  if (opt == "yesno") {
    return { Button::Yes, Button::No, Button::Yes };
  }
  if (opt == "yesnocancel") {
    return { Button::Yes, Button::No, Button::Cancel };
  }
  return { Button::NoButton, Button::NoButton, Button::NoButton };
}

QMessageBox::Icon
getMessageBoxIcon(string_view opt)
{
  using Icon = QMessageBox::Icon;
  if (opt == "!") {
    return Icon::Warning;
  }
  if (opt == "?") {
    return Icon::Question;
  }
  if (opt == "i") {
    return Icon::Information;
  }
  if (opt == ".") {
    return Icon::Critical;
  }
  return Icon::NoIcon;
}

template<typename T>
inline void
splitLua(lua_State* L,
         string_view input,
         T sep,
         size_t sepSize,
         lua_Integer max)
{
  size_t last = 0;
  size_t next = 0;
  lua_Integer i = 1;
  for (; i <= max && (next = input.find(sep, last)) != string_view::npos; ++i) {
    push(L, input.substr(last, next - last));
    lua_rawseti(L, -2, i);
    last = next + sepSize;
  }
  if (string_view rest = input.substr(last); !rest.empty()) {
    push(L, rest);
    lua_rawseti(L, -2, i);
  }
}
} // namespace

namespace {
int
L_activatenotepad(lua_State* L)
{
  expectMaxArgs(L, 1);
  Notepad* notepad = getApi(L).globalNotepad(getQString(L, 1));
  if (notepad == nullptr) {
    push(L, false);
    return 1;
  }
  notepad->activateWindow();
  push(L, true);
  return 1;
}

int
L_appendtonotepad(lua_State* L)
{
  expectMaxArgs(L, 3);
  Notepad* notepad = getApi(L).globalNotepad(getQString(L, 1, {}));
  const QString& message = getQString(L, 2);
  const bool replace = getBool(L, 3, false);
  if (notepad == nullptr) {
    push(L, false);
    return 1;
  }
  if (replace) {
    notepad->setText(message);
  } else {
    notepad->appendText(message);
  }
  push(L, true);
  return 1;
}

int
L_callbackslist(lua_State* L)
{
  expectMaxArgs(L, 0);
  const span callbacks = NamedPluginCallback::list();
  lua_createtable(L, static_cast<int>(callbacks.size()), 0);
  int i = -1;
  for (const NamedPluginCallback* callback : callbacks) {
    push(L, callback->name());
    lua_rawseti(L, -2, ++i);
  }
  return 1;
}

int
L_choose(lua_State* L)
{
  expectMaxArgs(L, 4);
  const QString message = getQString(L, 1, {});
  const QString title = getQString(L, 2, QCoreApplication::applicationName());
  const QVariant defaultKey = getKey(L, 4, 4);

  unique_ptr<ChooseDialog> dialog =
    make_unique<ChooseDialog>(title, message, getApi(L).parentWidget());
  return execScriptDialog(L, 3, *dialog, defaultKey);
}

// NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
int
L_compress(lua_State* L)
{
  expectMaxArgs(L, 2);
  const QByteArrayView bytes = getBytes(L, 1);
  const lua_Integer level = getInteger(L, 2, 6);
  luaL_argexpected(L, level >= 0 && level <= 9, 2, "integer between 0 and 9");
  push(L,
       qCompress(reinterpret_cast<const uchar*>(bytes.data()),
                 static_cast<int>(bytes.size()),
                 static_cast<int>(level)));
  return 1;
}

int
L_decompress(lua_State* L)
{
  expectMaxArgs(L, 1);
  const QByteArrayView bytes = getBytes(L, 1);
  push(L,
       qUncompress(reinterpret_cast<const uchar*>(bytes.data()),
                   static_cast<int>(bytes.size())));
  return 1;
}
// NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

int
L_directorypicker(lua_State* L)
{
  expectMaxArgs(L, 2);
  const QString path = QFileDialog::getExistingDirectory(
    getApi(L).parentWidget(), getQString(L, 1, {}), getQString(L, 2, {}));
  if (path.isEmpty()) {
    lua_pushnil(L);
  } else {
    push(L, path);
  }
  return 1;
}

int
L_editbox(lua_State* L)
{
  unique_ptr<InputBox> dialog =
    make_unique<InputBox>(L, true, getApi(L).parentWidget());
  if (dialog->exec() == QDialog::Accepted) {
    push(L, dialog->textValue());
  } else {
    lua_pushnil(L);
  }
  return 1;
}

int
L_filepicker(lua_State* L)
{
  expectMaxArgs(L, 5);
  const QString title = getQString(L, 1, {});
  const QString defaultName = getQString(L, 2, {});
  const QString extension = getQString(L, 3, {});
  const bool isSave = getBool(L, 5, false);
  QWidget* parent = getApi(L).parentWidget();
  const QString path =
    isSave ? QFileDialog::getSaveFileName(parent, title, defaultName)
           : QFileDialog::getOpenFileName(parent, title, defaultName);
  if (path.isEmpty()) {
    lua_pushnil(L);
  } else {
    push(L, path);
  }
  return 1;
}

int
L_fontpicker(lua_State* L)
{
  expectMaxArgs(L, 3);
  QFont initialFont = getQFont(L, 1, {});
  const int pointSize = getInt(L, 2, -1);
  const QColor initialColor = getQColor(L, 3, Qt::GlobalColor::black);
  if (pointSize != -1) {
    initialFont.setPointSize(pointSize);
  }
  QWidget* parent = getApi(L).parentWidget();
  bool ok;
  QFont font = initialFont.family().isEmpty()
                 ? QFontDialog::getFont(&ok, parent)
                 : QFontDialog::getFont(&ok, initialFont, parent);
  if (!ok) {
    lua_pushnil(L);
    return 1;
  }
  const QColor color = QColorDialog::getColor(initialColor, parent);
  if (!color.isValid()) {
    lua_pushnil(L);
    return 1;
  }
  lua_createtable(L, 0, 9);
  pushEntry(L, "name", font.family());
  pushEntry(L, "size", font.pointSize());
  pushEntry(L, "colour", color);
  pushEntry(L, "charset", 1);
  pushEntry(L, "underline", font.underline());
  pushEntry(L, "strikeout", font.strikeOut());
  pushEntry(L, "italic", font.italic());
  pushEntry(L, "bold", font.bold());
  pushEntry(L, "style", font.styleName());
  return 1;
}

int
L_fromhex(lua_State* L)
{
  expectMaxArgs(L, 1);
  push(L, QByteArray::fromHex(QByteArray(getBytes(L, 1))));
  return 1;
}

int
L_getfontfamilies(lua_State* L)
{
  expectMaxArgs(L, 0);
  const QStringList families = QFontDatabase::families();
  lua_createtable(L, 0, static_cast<int>(families.size()));
  for (const QString& family : families) {
    if (family.startsWith(u'.')) {
      continue;
    }
    pushEntry(L, family, true);
  }
  return 1;
}

int
L_hash(lua_State* L)
{
  expectMaxArgs(L, 1);
  doHash(L, QCryptographicHash::Algorithm::Sha1);
  return 1;
}

int
L_info(lua_State* L)
{
  expectMaxArgs(L, 0);
  Settings settings;
  lua_createtable(L, 0, 10);
  const QFont fixedFont =
    QFontDatabase::systemFont(QFontDatabase::SystemFont::FixedFont);
  pushEntry(L, "current_directory", QDir::currentPath());
  pushEntry(L, "app_directory", QCoreApplication::applicationDirPath());
  pushEntry(L, "world_files_directory", settings.getWorldsDir());
  pushEntry(L, "log_files_directory", settings.getLogsDir());
  pushEntry(L, "plugins_directory", settings.getPluginsDir());
  pushEntry(L, "startup_directory", settings.getStartupDirectoryOrDefault());
  pushEntry(L, "locale", QLocale().name());
  pushEntry(L, "fixed_pitch_font", fixedFont.family());
  pushEntry(L, "fixed_pitch_font_size", fixedFont.pointSizeF());
  pushEntry(L, "translator_file", QTranslator().filePath());
  return 1;
}

int
L_infotypes(lua_State* L)
{
  lua_createtable(L, maxInfoType, 0);
  for (const auto& entry : infoTypes) {
    push(L, entry.second);
    lua_rawseti(L, -2, entry.first);
  }
  return 1;
}

int
L_inputbox(lua_State* L)
{
  unique_ptr<InputBox> dialog =
    make_unique<InputBox>(L, false, getApi(L).parentWidget());
  if (dialog->exec() == QDialog::Accepted) {
    push(L, dialog->textValue());
  } else {
    lua_pushnil(L);
  }
  return 1;
}

int
L_listbox(lua_State* L)
{
  expectMaxArgs(L, 4);
  const QString message = getQString(L, 1, {});
  const QString title = getQString(L, 2, QCoreApplication::applicationName());
  const QVariant defaultKey = getKey(L, 4, 4);

  unique_ptr<ListBoxDialog> dialog =
    make_unique<ListBoxDialog>(title, message, getApi(L).parentWidget());
  return execScriptDialog(L, 3, *dialog, defaultKey);
}

int
L_md5(lua_State* L)
{
  expectMaxArgs(L, 1);
  doHash(L, QCryptographicHash::Algorithm::Md5);
  return 1;
}

int
L_msgbox(lua_State* L)
{
  using Button = QMessageBox::StandardButton;

  expectMaxArgs(L, 5);
  const QString message = getQString(L, 1);
  const QString title = getQString(L, 2, QCoreApplication::applicationName());
  const array<Button, 3> buttonArray =
    getMessageBoxButtons(getString(L, 3, "ok"));
  const QMessageBox::Icon icon = getMessageBoxIcon(getString(L, 4, "!"));
  const lua_Integer defaultButton = getInteger(L, 5, 1);

  const QMessageBox::StandardButtons buttons =
    buttonArray[0] | buttonArray[1] | buttonArray[2];
  luaL_argcheck(L, buttons != Button::NoButton, 3, "msgbox type unknown");
  luaL_argcheck(L, icon != QMessageBox::Icon::NoIcon, 4, "msgbox type unknown");
  luaL_argcheck(L,
                defaultButton >= 1 && defaultButton <= 3,
                5,
                "msgbox default button must be 1, 2 or 3");
  QWidget* parent = getApi(L).parentWidget();
  unique_ptr<QMessageBox> messageBox =
    make_unique<QMessageBox>(icon, title, message, buttons, parent);
  messageBox->setDefaultButton(buttonArray.at(defaultButton - 1));
  push(L, formatStandardButton(Button(messageBox->exec())));
  return 1;
}

int
L_multilistbox(lua_State* L)
{
  expectMaxArgs(L, 4);
  const QString message = getQString(L, 1, {});
  const QString title = getQString(L, 2, QCoreApplication::applicationName());
  const int defaultType = lua_type(L, 3);
  luaL_argexpected(L,
                   defaultType == LUA_TTABLE || defaultType == LUA_TNIL ||
                     defaultType == LUA_TNONE,
                   3,
                   "table");

  QList<QVariant> defaults;
  if (defaultType == LUA_TTABLE) {
    defaults.reserve(luaL_len(L, 4));

    for (lua_pushnil(L); lua_next(L, 4); lua_pop(L, 1)) {
      defaults.push_back(getKey(L, 4, -2));
    }
  }

  unique_ptr<ListBoxDialog> dialog =
    make_unique<ListBoxDialog>(title, message, getApi(L).parentWidget());
  dialog->setMode(QListWidget::SelectionMode::MultiSelection);
  return execScriptDialog(L, 3, *dialog, defaults);
}

int
L_rgb(lua_State* L)
{
  expectMaxArgs(L, 3);
  push(L, getInteger(L, 1) | getInteger(L, 2) << 8 | getInteger(L, 3) << 16);
  return 1;
}

int
L_sha256(lua_State* L)
{
  expectMaxArgs(L, 1);
  doHash(L, QCryptographicHash::Algorithm::Sha256);
  return 1;
}

int
L_split(lua_State* L)
{
  expectMaxArgs(L, 3);
  const string_view input = getString(L, 1);
  const string_view sep = getString(L, 2);
  const lua_Integer count = getInteger(L, 3, 0);
  if (sep.empty()) {
    lua_pushliteral(L, "Separator must not be an empty string");
    lua_error(L);
  }
  if (count < 0) {
    lua_pushliteral(L, "Count must be positive or zero");
    lua_error(L);
  }

  lua_newtable(L);
  const lua_Integer max = count == 0 ? INT_MAX : count;

  const size_t sepSize = sep.size();
  if (sepSize == 1) {
    splitLua(L, input, sep[0], 1, max);
  } else {
    splitLua(L, input, sep, sepSize, max);
  }

  return 1;
}

int
L_timer(lua_State* L)
{
  expectMaxArgs(L, 0);
  QElapsedTimer timer;
  timer.start();
  push(L, timer.msecsSinceReference());
  return 1;
}

int
L_tohex(lua_State* L)
{
  expectMaxArgs(L, 1);
  push(L, QByteArray(getBytes(L, 1)).toHex());
  return 1;
}

int
L_utf8decode(lua_State* L)
{
  expectMaxArgs(L, 1);
  const string_view text = getString(L, 1);
  if (text.empty()) {
    lua_createtable(L, 0, 0);
    return 1;
  }
  const rust::Vec<uint32_t> codes = ffi::util::decode_utf8(text);
  if (codes.empty()) {
    lua_pushnil(L);
    return 1;
  }
  pushList(L, codes);
  return 1;
}

int
L_utf8encode(lua_State* L)
{
  const int n = lua_gettop(L);
  rust::String buf;
  for (int i = 1; i <= n; ++i) {
    switch (lua_type(L, i)) {
      case LUA_TTABLE:
        break;
      case LUA_TNUMBER:
        encodeUtf8Char(L, buf, i);
        break;
      default:
        luaL_typeerror(L, i, "integer or table");
    }
    for (int j = 1, ttype; (ttype = lua_geti(L, i, j)) != LUA_TNONE; ++j) {
      if (ttype != LUA_TNUMBER) {
        luaL_error(L,
                   "Index [%d] of table at argument #%d to 'utf8encode' is "
                   "not a number",
                   j,
                   i);
      }
      encodeUtf8Char(L, buf, -1);
      lua_pop(L, 1);
    }
    lua_pop(L, 1);
  }
  push(L, buf);
  return 1;
}

int
L_utf8sub(lua_State* L)
{
  expectMaxArgs(L, 3);
  size_t errorPos = SIZE_MAX;
  const rust::Str s = ffi::util::utf8_substring(
    getString(L, 1), getInteger(L, 2), getInteger(L, 3, -1), errorPos);

  if (errorPos != SIZE_MAX) {
    lua_pushnil(L);
    push(L, errorPos);
    return 2;
  }
  push(L, s);
  return 1;
}

int
L_utf8valid(lua_State* L)
{
  expectMaxArgs(L, 1);
  push(L, ffi::util::is_utf8_valid(getString(L, 1)));
  return 1;
}

int
L_noop_nil(lua_State* L)
{
  lua_pushnil(L);
  return 1;
}
} // namespace

static const struct luaL_Reg utilslib[] = {
  { "activatenotepad", L_activatenotepad },
  { "appendtonotepad", L_appendtonotepad },
  { "callbackslist", L_callbackslist },
  { "choose", L_choose },
  { "compress", L_compress },
  { "decompress", L_decompress },
  { "directorypicker", L_directorypicker },
  { "editbox", L_editbox },
  { "fontpicker", L_fontpicker },
  { "fromhex", L_fromhex },
  { "getfontfamilies", L_getfontfamilies },
  { "glyph_available", L_noop_nil },
  { "hash", L_hash },
  { "info", L_info },
  { "infotypes", L_infotypes },
  { "inputbox", L_inputbox },
  { "filepicker", L_filepicker },
  { "listbox", L_listbox },
  { "md5", L_md5 },
  { "msgbox", L_msgbox },
  { "multilistbox", L_multilistbox },
  { "reload_global_prefs", L_noop_nil },
  { "rgb", L_rgb },
  { "sendtofront", L_noop_nil },
  { "sha256", L_sha256 },
  { "split", L_split },
  { "timer", L_timer },
  { "tohex", L_tohex },
  { "umsgbox", L_msgbox },
  { "utf8decode", L_utf8decode },
  { "utf8encode", L_utf8encode },
  { "utf8sub", L_utf8sub },
  { "utf8valid", L_utf8valid },
  { nullptr, nullptr }
};

namespace {
int
L_utils_newindex(lua_State* L)
{
  lua_pushliteral(L, "attempt to update a read-only table");
  lua_error(L);
  return 0;
}
} // namespace

static const struct luaL_Reg utils_meta[] = { { "__newindex",
                                                L_utils_newindex },
                                              { nullptr, nullptr } };

int
luaopen_utils(lua_State* L)
{
  luaL_newlib(L, utilslib);
  luaopen_base64(L);
  lua_getfield(L, -1, "decode");
  lua_setfield(L, -3, "base64decode");
  lua_getfield(L, -1, "encode");
  lua_setfield(L, -3, "base64encode");
  lua_pop(L, 1);

  luaL_newmetatable(L, utilsRegKey);
  luaL_setfuncs(L, utils_meta, 0);
  lua_setmetatable(L, -2);

  return 1;
}
