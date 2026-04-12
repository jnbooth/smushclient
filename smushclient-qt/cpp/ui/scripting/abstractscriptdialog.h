#pragma once
#include <QtWidgets/QDialog>

struct lua_State;

class AbstractScriptDialog : public QDialog
{
  Q_OBJECT

public:
  explicit AbstractScriptDialog(QWidget* parent)
    : QDialog(parent) {};

  virtual void addItem(const QString& text,
                       const QVariant& value,
                       bool active) = 0;
  virtual int pushValue(lua_State* L) const = 0;
  virtual void sortItems() = 0;
};
