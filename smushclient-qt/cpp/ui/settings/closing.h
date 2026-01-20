#pragma once
#include <QtWidgets/QWidget>

namespace Ui {
class SettingsClosing;
} // namespace Ui

class Settings;

class SettingsClosing : public QWidget
{
  Q_OBJECT

public:
  explicit SettingsClosing(Settings& settings, QWidget* parent = nullptr);
  ~SettingsClosing() override;

private:
  Ui::SettingsClosing* ui;
};
