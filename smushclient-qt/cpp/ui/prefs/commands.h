#pragma once
#include <QtWidgets/QWidget>
#include "cxx-qt-gen/ffi.cxxqt.h"

namespace Ui
{
  class PrefsCommands;
}

class PrefsCommands : public QWidget
{
  Q_OBJECT

public:
  explicit PrefsCommands(World &world, QWidget *parent = nullptr);
  ~PrefsCommands();

private:
  Ui::PrefsCommands *ui;
};
