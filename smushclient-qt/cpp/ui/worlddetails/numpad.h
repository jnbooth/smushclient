#pragma once
#include <QtWidgets/QWidget>

namespace Ui
{
  class PrefsNumpad;
}

class World;

class PrefsNumpad : public QWidget
{
  Q_OBJECT

public:
  explicit PrefsNumpad(const World &world, QWidget *parent = nullptr);
  ~PrefsNumpad();

private:
  Ui::PrefsNumpad *ui;

private slots:
  void on_show_mod_toggled(bool checked);
};
