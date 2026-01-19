#pragma once
#include <QtWidgets/QWidget>

namespace Ui {
class SettingsConnecting;
} // namespace Ui

class Settings;

class SettingsConnecting : public QWidget
{
  Q_OBJECT

public:
  explicit SettingsConnecting(Settings& settings, QWidget* parent = nullptr);
  ~SettingsConnecting();

private:
  Ui::SettingsConnecting* ui;
};
