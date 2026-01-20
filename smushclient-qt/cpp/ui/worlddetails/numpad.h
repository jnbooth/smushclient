#pragma once
#include <QtWidgets/QWidget>

namespace Ui {
class PrefsNumpad;
} // namespace Ui

class World;

class PrefsNumpad : public QWidget
{
  Q_OBJECT

public:
  explicit PrefsNumpad(const World& world, QWidget* parent = nullptr);
  ~PrefsNumpad() override;

private slots:
  void on_show_mod_toggled(bool checked);

private:
  Ui::PrefsNumpad* ui;
};
