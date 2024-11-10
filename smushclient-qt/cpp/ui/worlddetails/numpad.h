#pragma once
#include <QtWidgets/QWidget>
#include "cxx-qt-gen/ffi.cxxqt.h"

namespace Ui
{
  class PrefsNumpad;
}

class PrefsNumpad : public QWidget
{
  Q_OBJECT

public:
  explicit PrefsNumpad(const World &world, QWidget *parent = nullptr);
  ~PrefsNumpad();

  private slots:
  void on_show_mod_toggled(bool checked);

  private:
  Ui::PrefsNumpad *ui;
};
