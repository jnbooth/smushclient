#pragma once
#include <QtWidgets/QWidget>
#include "cxx-qt-gen/ffi.cxxqt.h"

namespace Ui
{
  class PrefsLogin;
}

class PrefsLogin : public QWidget
{
  Q_OBJECT

public:
  explicit PrefsLogin(World &world, QWidget *parent = nullptr);
  ~PrefsLogin();

private slots:
  void on_ConnectText_textChanged();

private:
  Ui::PrefsLogin *ui;
  World &world;
};
