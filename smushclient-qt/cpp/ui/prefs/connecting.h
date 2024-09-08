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
  explicit PrefsConnecting(World &world, QWidget *parent = nullptr);
  ~PrefsConnecting();

private slots:
  void on_ConnectText_textChanged();

private:
  Ui::PrefsConnecting *ui;
  World &world;
};
