#include "inputbox.h"
#include "../../casting.h"
#include "../../scripting/qlua.h"
#include <QtCore/QCoreApplication>
extern "C"
{
#include "lauxlib.h"
}

using qlua::expectMaxArgs;
using qlua::getNumber;
using qlua::getQFont;
using qlua::getQString;
using qlua::push;

// Private utils

struct InputBoxExtras
{
  explicit InputBoxExtras(lua_State* L, int idx)
  {
    switch (lua_type(L, idx)) {
      case LUA_TNIL:
      case LUA_TNONE:
        return;
      case LUA_TTABLE:
        break;
      [[unlikely]] default:
        luaL_typeerror(L, idx, "table");
    }
    int top = lua_gettop(L);
    if (lua_getfield(L, idx, "validate") == LUA_TFUNCTION) {
      validate = true;
      ++top;
    }
    if (lua_getfield(L, idx, "max_length") == LUA_TNUMBER) {
      int isInt;
      const lua_Integer len = lua_tointegerx(L, -1, &isInt);
      if (isInt) {
        max_length = clamped_cast<int>(len);
      }
    }
    if (lua_getfield(L, idx, "ok_button") == LUA_TSTRING) {
      ok_button = QString::fromUtf8(lua_tobytes(L, -1));
    }
    if (lua_getfield(L, idx, "cancel_button") == LUA_TSTRING) {
      cancel_button = QString::fromUtf8(lua_tobytes(L, -1));
    }
    if (lua_getfield(L, idx, "read_only") == LUA_TBOOLEAN) {
      read_only = lua_tobool(L, -1);
    }
    if (lua_getfield(L, idx, "no_default") == LUA_TBOOLEAN) {
      read_only = lua_tobool(L, -1);
    }
    lua_settop(L, top);
  }

  int max_length = INT_MAX;
  bool validate = false;
  QString ok_button;
  QString cancel_button;
  bool read_only = false;
  bool no_default = false;
};

// Public methods

InputBox::InputBox(lua_State* L, bool multiline, QWidget* parent)
  : QInputDialog(parent)
  , L(L)
{
  expectMaxArgs(L, 6);
  const QString message = getQString(L, 1, {});
  const QString title = getQString(L, 2, QCoreApplication::applicationName());
  const QString defaultText = getQString(L, 3, {});
  QFont font = getQFont(L, 4, {});
  const qreal fontSize = getNumber(L, 5, -1);
  if (fontSize > 0) {
    font.setPointSizeF(fontSize);
  }
  InputBoxExtras extras(L, 6);

  hasValidate = extras.validate;

  setLabelText(message);
  setWindowTitle(title);
  if (!font.family().isEmpty()) {
    setFont(font);
  }
  if (!extras.ok_button.isEmpty()) {
    setOkButtonText(extras.ok_button);
  }
  if (!extras.cancel_button.isEmpty()) {
    setCancelButtonText(extras.cancel_button);
  }
  if (extras.read_only) {
  }
  if (!extras.no_default) {
    setTextValue(defaultText);
  }
  setOption(InputDialogOption::UsePlainTextEditForTextInput, multiline);
}

// Public overrides

void
InputBox::done(int result)
{
  if (result != QDialog::Accepted || validate()) {
    QInputDialog::done(result);
    return;
  }
}

// Private methods

bool
InputBox::validate() const
{
  if (!hasValidate) {
    return true;
  }
  lua_pushvalue(L, -1);
  push(L, textValue());
  lua_call(L, 1, 1);
  const bool valid = lua_tobool(L, -1);
  lua_pop(L, 1);
  return valid;
}
