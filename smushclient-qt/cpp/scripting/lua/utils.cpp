#include "utils.h"
#include "../../ui/scripting/choose.h"
#include "../../ui/scripting/listbox.h"
#include "../qlua.h"
#include <QtCore/QCryptographicHash>
#include <QtGui/QFontDatabase>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QInputDialog>
extern "C"
{
#include "lauxlib.h"
  LUALIB_API int luaopen_base64(lua_State* L);
}

using std::string_view;

using qlua::expectMaxArgs;
using qlua::push;
using qlua::pushEntry;
using qlua::pushQVariant;

namespace {
const char* const utilsRegKey = "smushclient.utils";
} // namespace

namespace {
void
doHash(lua_State* L, QCryptographicHash::Algorithm algorithm)
{
  push(L, QCryptographicHash::hash(qlua::getBytes(L, 1), algorithm).toHex());
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
  lua_pushnil(L); // first key
  size_t size;

  while (lua_next(L, idx) != 0) {
    const QVariant key = qlua::getQVariant(L, -2);
    const char* data = lua_tolstring(L, -1, &size);
    const QString value = QString::fromUtf8(data, static_cast<qsizetype>(size));
    dialog.addItem(value, key, isOptionSelected(key, selection));
    lua_pop(L, 1);
  }

  dialog.sortItems();

  if (dialog.exec() == QDialog::Accepted) {
    pushQVariant(L, dialog.value());
  } else {
    lua_pushnil(L);
  }

  return 1;
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
L_choose(lua_State* L)
{
  expectMaxArgs(L, 4);
  const QString message = qlua::getQString(L, 1, QString());
  const QString title =
    qlua::getQString(L, 2, QCoreApplication::applicationName());
  luaL_argexpected(L, lua_type(L, 3) == LUA_TTABLE, 3, "table");
  const QVariant defaultKey = qlua::getQVariant(L, 4);

  Choose dialog(title, message);
  return execScriptDialog(L, 3, dialog, defaultKey);
}

int
L_compress(lua_State* L)
{
  expectMaxArgs(L, 2);
  const QByteArrayView bytes = qlua::getBytes(L, 1);
  const lua_Integer level = qlua::getInteger(L, 2, 6);
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
  const QByteArrayView bytes = qlua::getBytes(L, 1);
  push(L,
       qUncompress(reinterpret_cast<const uchar*>(bytes.data()),
                   static_cast<int>(bytes.size())));
  return 1;
}

int
L_directorypicker(lua_State* L)
{
  expectMaxArgs(L, 2);
  const QString title = qlua::getQString(L, 1, QString());
  const QString defaultName = qlua::getQString(L, 2, QString());
  const QString path =
    QFileDialog::getExistingDirectory(nullptr, title, defaultName);
  if (path.isEmpty()) {
    lua_pushnil(L);
  } else {
    push(L, path);
  }
  return 1;
}

int
L_filepicker(lua_State* L)
{
  expectMaxArgs(L, 5);
  const QString title = qlua::getQString(L, 1, QString());
  const QString defaultName = qlua::getQString(L, 2, QString());
  const QString extension = qlua::getQString(L, 3, QString());
  const bool isSave = qlua::getBool(L, 5, false);
  const QString path =
    isSave ? QFileDialog::getSaveFileName(nullptr, title, defaultName)
           : QFileDialog::getOpenFileName(nullptr, title, defaultName);
  if (path.isEmpty()) {
    lua_pushnil(L);
  } else {
    push(L, path);
  }
  return 1;
}

int
L_fromhex(lua_State* L)
{
  expectMaxArgs(L, 1);
  push(L, QByteArray::fromHex(QByteArray(qlua::getBytes(L, 1))));
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
L_getsystemfont(lua_State* L)
{
  expectMaxArgs(L, 1);
  const lua_Integer font =
    qlua::getInteger(L, 1, QFontDatabase::SystemFont::FixedFont);
  if (font < QFontDatabase::SystemFont::GeneralFont ||
      font > QFontDatabase::SystemFont::SmallestReadableFont) {
    lua_pushnil(L);
    return 1;
  }
  const QFont systemFont =
    QFontDatabase::systemFont(static_cast<QFontDatabase::SystemFont>(font));
  push(L, systemFont.family());
  lua_pushinteger(L, systemFont.pointSize());
  return 2;
}

int
L_hash(lua_State* L)
{
  expectMaxArgs(L, 1);
  doHash(L, QCryptographicHash::Algorithm::Sha1);
  return 1;
}

int
L_inputbox(lua_State* L)
{
  expectMaxArgs(L, 6);
  const QString message = qlua::getQString(L, 1, QString());
  const QString title =
    qlua::getQString(L, 2, QCoreApplication::applicationName());
  const QString defaultText = qlua::getQString(L, 3, QString());
  QFont font = qlua::getQFont(L, 4, QString());
  const qreal fontSize = qlua::getInt(L, 5, -1);
  if (fontSize > 0) {
    font.setPointSizeF(fontSize);
  }

  QInputDialog dialog;
  dialog.setWindowTitle(title);
  dialog.setLabelText(message);
  dialog.setTextValue(defaultText);
  if (!font.family().isEmpty()) {
    dialog.setFont(font);
  }

  if (dialog.exec() == QDialog::Accepted) {
    push(L, dialog.textValue());
  } else {
    lua_pushnil(L);
  }

  return 1;
}

int
L_listbox(lua_State* L)
{
  expectMaxArgs(L, 4);
  const QString message = qlua::getQString(L, 1, QString());
  const QString title =
    qlua::getQString(L, 2, QCoreApplication::applicationName());
  luaL_argexpected(L, lua_type(L, 3) == LUA_TTABLE, 3, "table");
  const QVariant defaultKey = qlua::getQVariant(L, 4);

  ListBox dialog(title, message);
  return execScriptDialog(L, 3, dialog, defaultKey);
}

int
L_md5(lua_State* L)
{
  expectMaxArgs(L, 1);
  doHash(L, QCryptographicHash::Algorithm::Md5);
  return 1;
}

int
L_multilistbox(lua_State* L)
{
  expectMaxArgs(L, 4);
  const QString message = qlua::getQString(L, 1, QString());
  const QString title =
    qlua::getQString(L, 2, QCoreApplication::applicationName());
  luaL_argexpected(L, lua_type(L, 3) == LUA_TTABLE, 3, "table");
  const int defaultType = lua_type(L, 3);
  luaL_argexpected(L,
                   defaultType == LUA_TTABLE || defaultType == LUA_TNIL ||
                     defaultType == LUA_TNONE,
                   3,
                   "table");

  QList<QVariant> defaults;
  if (defaultType == LUA_TTABLE) {
    defaults.reserve(static_cast<qsizetype>(lua_rawlen(L, 4)));
    lua_pushnil(L); // first key

    while (lua_next(L, 4) != 0) {
      defaults.push_back(qlua::getQVariant(L, -2));
      lua_pop(L, 1);
    }
  }

  ListBox dialog(title, message);
  dialog.setMode(QListWidget::SelectionMode::MultiSelection);
  return execScriptDialog(L, 3, dialog, defaults);
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
  const string_view input = qlua::getString(L, 1);
  const string_view sep = qlua::getString(L, 2);
  const lua_Integer count = qlua::getInteger(L, 3, 0);
  if (sep.empty()) {
    push(L, "Separator must not be an empty string");
    lua_error(L);
  }
  if (count < 0) {
    push(L, "Count must be positive or zero");
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
L_tohex(lua_State* L)
{
  expectMaxArgs(L, 1);
  push(L, QByteArray(qlua::getBytes(L, 1)).toHex());
  return 1;
}

} // namespace

static const struct luaL_Reg utilslib[] = {
  { "choose", L_choose },
  { "compress", L_compress },
  { "decompress", L_decompress },
  { "directorypicker", L_directorypicker },
  { "fromhex", L_fromhex },
  { "getfontfamilies", L_getfontfamilies },
  { "getsystemfont", L_getsystemfont },
  { "hash", L_hash },
  { "inputbox", L_inputbox },
  { "filepicker", L_filepicker },
  { "listbox", L_listbox },
  { "md5", L_md5 },
  { "multilistbox", L_multilistbox },
  { "sha256", L_sha256 },
  { "split", L_split },
  { "tohex", L_tohex },
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
