#pragma once
#include <QtWidgets/QWidget>
#include "cxx-qt-gen/ffi.cxxqt.h"

namespace Ui
{
  class PrefsColor;
}

class PrefsColor : public QWidget
{
  Q_OBJECT

public:
  explicit PrefsColor(const World &world, QWidget *parent = nullptr);
  ~PrefsColor();

private:
  Ui::PrefsColor *ui;
};
