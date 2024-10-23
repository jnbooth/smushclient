#include "luaapi.h"
#include <QtGui/QFontDatabase>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QInputDialog>
#include "qlua.h"
#include "scriptapi.h"
extern "C"
{
#include "lauxlib.h"
  LUALIB_API int luaopen_base64(lua_State *L);
}

using std::optional;
using std::string_view;

using qlua::expectMaxArgs;

static int L_split(lua_State *L)
{
  expectMaxArgs(L, 3);
  size_t inputLength;
  const char *input = luaL_checklstring(L, 1, &inputLength);
  size_t sepLength;
  const char *sep = luaL_checklstring(L, 2, &sepLength);
  const int count = qlua::getInt(L, 3, 0);
  if (sepLength != 1)
  {
    qlua::pushString(L, "Separator must be a single character");
    lua_error(L);
  }
  if (count < 0)
  {
    qlua::pushString(L, "Count must be positive or zero");
    lua_error(L);
  }

  lua_newtable(L);
  const int max = count == 0 ? INT_MAX : count;

  const char *endPtr = input + inputLength;
  const char *sepPtr;
  int i = 1;
  for (; i <= max && input < endPtr && (sepPtr = strchr(input, *sep)) != NULL; ++i)
  {
    lua_pushlstring(L, input, sepPtr - input);
    lua_rawseti(L, -2, i);
    input = sepPtr + 1;
  }
  if (input < endPtr)
  {
    lua_pushlstring(L, input, endPtr - input);
    lua_rawseti(L, -2, i);
  }
  return 1;
}

static int L_directorypicker(lua_State *L)
{
  expectMaxArgs(L, 2);
  const QString title = qlua::getQString(L, 1, QString());
  const QString defaultName = qlua::getQString(L, 2, QString());
  const QString path = QFileDialog::getExistingDirectory(nullptr, title, defaultName);
  if (path.isEmpty())
    lua_pushnil(L);
  else
    qlua::pushQString(L, path);
  return 1;
}

static int L_filepicker(lua_State *L)
{
  expectMaxArgs(L, 5);
  const QString title = qlua::getQString(L, 1, QString());
  const QString defaultName = qlua::getQString(L, 2, QString());
  const QString extension = qlua::getQString(L, 3, QString());
  const bool isSave = qlua::getBool(L, 5, false);
  const QString path =
      isSave
          ? QFileDialog::getSaveFileName(nullptr, title, defaultName)
          : QFileDialog::getOpenFileName(nullptr, title, defaultName);
  if (path.isEmpty())
    lua_pushnil(L);
  else
    qlua::pushQString(L, path);
  return 1;
}

static int L_getfontfamilies(lua_State *L)
{
  expectMaxArgs(L, 0);
  const QStringList families = QFontDatabase::families();
  lua_createtable(L, 0, families.size());
  for (const QString &family : families)
  {
    qlua::pushQString(L, family);
    lua_pushboolean(L, true);
    lua_rawset(L, -3);
  }
  return 1;
}

static int L_inputbox(lua_State *L)
{
  expectMaxArgs(L, 6);
  const QString message = qlua::getQString(L, 1, QString());
  const QString title = qlua::getQString(L, 2, QStringLiteral("SmushClient"));
  const QString defaultText = qlua::getQString(L, 3, QString());
  const QString fontFamily = qlua::getQString(L, 4, QString());
  const int fontSize = qlua::getInt(L, 5, -1);

  QInputDialog *dialog = new QInputDialog;
  dialog->setWindowTitle(title);
  dialog->setLabelText(message);
  dialog->setTextValue(defaultText);
  if (!fontFamily.isEmpty())
    dialog->setFont(QFont(fontFamily, fontSize));

  if (dialog->exec() == QDialog::Accepted)
    qlua::pushQString(L, dialog->textValue());
  else
    lua_pushnil(L);

  return 1;
}

static int L_listbox(lua_State *L)
{
  expectMaxArgs(L, 4);
  const QString message = qlua::getQString(L, 1);
  const QString title = qlua::getQString(L, 2);
  luaL_argexpected(L, lua_type(L, 3) == LUA_TTABLE, 3, "table");
  const QVariant defaultKey = qlua::getQVariant(L, 4);

  QString defaultValue;
  QVariantHash choices;
  QStringList choiceValues;
  lua_pushnil(L); // first key
  size_t size;
  while (lua_next(L, 3) != 0)
  {
    const QVariant key = qlua::getQVariant(L, -2);
    const QString value = QString::fromUtf8(lua_tolstring(L, -1, &size), size);
    if (key == defaultKey)
      defaultValue = value;
    choices[value] = key;
    lua_pop(L, 1);
  }

  QInputDialog *dialog = new QInputDialog;
  dialog->setWindowTitle(title);
  dialog->setLabelText(message);
  dialog->setComboBoxItems(choiceValues);
  if (!defaultValue.isEmpty())
    dialog->setTextValue(defaultValue);

  if (dialog->exec() == QDialog::Accepted)
    qlua::pushQVariant(L, choices[dialog->textValue()]);
  else
    lua_pushnil(L);

  return 1;
}

static const struct luaL_Reg utilslib[] =
    {{"choose", L_listbox},
     {"directorypicker", L_directorypicker},
     {"getfontfamilies", L_getfontfamilies},
     {"inputbox", L_inputbox},
     {"filepicker", L_filepicker},
     {"listbox", L_listbox},
     {"split", L_split},

     {NULL, NULL}, // for base64.decode
     {NULL, NULL}, // for base64.encode
     {NULL, NULL}};

int luaopen_utils(lua_State *L)
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
