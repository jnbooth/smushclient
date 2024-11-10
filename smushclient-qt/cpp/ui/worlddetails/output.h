#pragma once
#include <QtWidgets/QWidget>
#include "cxx-qt-gen/ffi.cxxqt.h"

namespace Ui
{
  class PrefsOutput;
}

class PrefsOutput : public QWidget
{
  Q_OBJECT

public:
  explicit PrefsOutput(const World &world, QWidget *parent = nullptr);
  ~PrefsOutput();

private:
  Ui::PrefsOutput *ui;
};
