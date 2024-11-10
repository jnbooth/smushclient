#pragma once
#include <QtWidgets/QWidget>
#include "cxx-qt-gen/ffi.cxxqt.h"

namespace Ui
{
  class PrefsConnecting;
}

class PrefsConnecting : public QWidget
{
  Q_OBJECT

public:
  explicit PrefsConnecting(const World &world, QWidget *parent = nullptr);
  ~PrefsConnecting();

private:
  Ui::PrefsConnecting *ui;
};
