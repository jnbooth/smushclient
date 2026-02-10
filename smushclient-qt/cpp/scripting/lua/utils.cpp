#include "utils.h"
#include "../../ui/scripting/choose.h"
#include "../../ui/scripting/listbox.h"
#include "../qlua.h"
#include <QtGui/QFontDatabase>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QInputDialog>
extern "C"
{
#include "lauxlib.h"
  LUALIB_API int luaopen_base64(lua_State* L);
}

using std::string_view;

using qlua::expectMaxArgs;

class SelectionPredicate
{
public:
  SelectionPredicate() = default;
  virtual ~SelectionPredicate() = default;

  SelectionPredicate(const SelectionPredicate&) = delete;
  SelectionPredicate& operator=(const SelectionPredicate&) = delete;
  SelectionPredicate& operator=(SelectionPredicate&&) = delete;

  SelectionPredicate(SelectionPredicate&& other) = delete;

  virtual bool isSelected(const QVariant& value) const = 0;
};

class SingleSelectionPredicate : public SelectionPredicate
{
public:
  explicit SingleSelectionPredicate(const QVariant& selection)
    : selection(selection)
  {
  }
  bool isSelected(const QVariant& value) const override
  {
    return value == selection;
  }

private:
  const QVariant& selection;
};

class MultiSelectionPredicate : public SelectionPredicate
{
public:
  explicit MultiSelectionPredicate(const QList<QVariant>& selection)
    : selection(selection)
  {
  }
  bool isSelected(const QVariant& value) const override
  {
    return selection.contains(value);
  }

private:
  const QList<QVariant>& selection;
};

namespace {
int
execScriptDialog(lua_State* L,
                 int idx,
                 AbstractScriptDialog& dialog,
                 const SelectionPredicate& pred)
{
  lua_pushnil(L); // first key
  size_t size;

  while (lua_next(L, idx) != 0) {
    const QVariant key = qlua::getQVariant(L, -2);
    const char* data = lua_tolstring(L, -1, &size);
    const QString value = QString::fromUtf8(data, static_cast<qsizetype>(size));
    dialog.addItem(value, key, pred.isSelected(key));
    lua_pop(L, 1);
  }

  dialog.sortItems();

  if (dialog.exec() == QDialog::Accepted) {
    qlua::pushQVariant(L, dialog.value());
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
    qlua::pushString(L, input.substr(last, next - last));
    lua_rawseti(L, -2, i);
    last = next + sepSize;
  }
  if (string_view rest = input.substr(last); !rest.empty()) {
    qlua::pushString(L, rest);
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

  SingleSelectionPredicate pred(defaultKey);
  Choose dialog(title, message);
  return execScriptDialog(L, 3, dialog, pred);
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
    qlua::pushQString(L, path);
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
    qlua::pushQString(L, path);
  }
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
    qlua::pushQString(L, family);
    qlua::pushBool(L, true);
    lua_rawset(L, -3);
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
  qlua::pushQString(L, systemFont.family());
  lua_pushinteger(L, systemFont.pointSize());
  return 2;
}

int
L_inputbox(lua_State* L)
{
  expectMaxArgs(L, 6);
  const QString message = qlua::getQString(L, 1, QString());
  const QString title =
    qlua::getQString(L, 2, QCoreApplication::applicationName());
  const QString defaultText = qlua::getQString(L, 3, QString());
  const QString fontFamily = qlua::getQString(L, 4, QString());
  const int fontSize = qlua::getInt(L, 5, -1);

  QInputDialog dialog;
  dialog.setWindowTitle(title);
  dialog.setLabelText(message);
  dialog.setTextValue(defaultText);
  if (!fontFamily.isEmpty()) {
    dialog.setFont(QFont(fontFamily, fontSize));
  }

  if (dialog.exec() == QDialog::Accepted) {
    qlua::pushQString(L, dialog.textValue());
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

  SingleSelectionPredicate pred(defaultKey);
  ListBox dialog(title, message);
  return execScriptDialog(L, 3, dialog, pred);
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

  MultiSelectionPredicate pred(defaults);
  ListBox dialog(title, message);
  dialog.setMode(QListWidget::SelectionMode::MultiSelection);
  return execScriptDialog(L, 3, dialog, pred);
}

int
L_split(lua_State* L)
{
  expectMaxArgs(L, 3);
  const string_view input = qlua::getString(L, 1);
  const string_view sep = qlua::getString(L, 2);
  const lua_Integer count = qlua::getInteger(L, 3, 0);
  if (sep.empty()) {
    qlua::pushString(L, "Separator must not be an empty string");
    lua_error(L);
  }
  if (count < 0) {
    qlua::pushString(L, "Count must be positive or zero");
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
} // namespace

static const struct luaL_Reg utilslib[] = {
  { "choose", L_choose },
  { "directorypicker", L_directorypicker },
  { "getfontfamilies", L_getfontfamilies },
  { "getsystemfont", L_getsystemfont },
  { "inputbox", L_inputbox },
  { "filepicker", L_filepicker },
  { "listbox", L_listbox },
  { "multilistbox", L_multilistbox },
  { "split", L_split },
  { nullptr, nullptr }
};

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
  return 1;
}
