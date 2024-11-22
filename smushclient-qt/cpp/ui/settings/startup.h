#pragma once
#include <QtWidgets/QWidget>

namespace Ui
{
  class SettingsStartup;
}

class Settings;

class SettingsStartup : public QWidget
{
  Q_OBJECT

public:
  explicit SettingsStartup(Settings &settings, QWidget *parent = nullptr);
  ~SettingsStartup();

private:
  Ui::SettingsStartup *ui;
  Settings &settings;

private slots:
  void on_OpenAtStartup_add_clicked();
  void on_OpenAtStartup_remove_clicked();
  void on_OpenAtStartup_up_clicked();
  void on_OpenAtStartup_down_clicked();
  void on_OpenAtStartup_itemSelectionChanged();
};
