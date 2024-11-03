#pragma once
#include <QtWidgets/QWidget>
#include "cxx-qt-gen/ffi.cxxqt.h"

namespace Ui
{
  class PrefsMud;
}

class PrefsMud : public QWidget
{
  Q_OBJECT

public:
  explicit PrefsMud(World &world, QWidget *parent = nullptr);
  ~PrefsMud();

private slots:
  void on_CommandStackCharacter_textChanged(const QString &character);

private:
  Ui::PrefsMud *ui;
  World &world;
};
