#pragma once
#include <QtWidgets/QWidget>
#include "cxx-qt-gen/ffi.cxxqt.h"

namespace Ui
{
  class PrefsAddress;
}

class PrefsAddress : public QWidget
{
  Q_OBJECT

public:
  explicit PrefsAddress(const World &world, QWidget *parent = nullptr);
  ~PrefsAddress();

private:
  Ui::PrefsAddress *ui;
};
