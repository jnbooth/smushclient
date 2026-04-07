#pragma once
#include <QtWidgets/QInputDialog>

struct lua_State;

class InputBox : public QInputDialog
{
  Q_OBJECT

public:
  explicit InputBox(lua_State* L, bool multiline, QWidget* parent = nullptr);

  void done(int result) override;

private:
  bool validate() const;

private:
  lua_State* L;
  bool hasValidate;
};
