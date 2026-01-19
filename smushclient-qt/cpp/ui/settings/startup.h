#pragma once
#include <QtWidgets/QWidget>

namespace Ui {
class SettingsStartup;
} // namespace Ui

class Settings;

class SettingsStartup : public QWidget
{
  Q_OBJECT

public:
  explicit SettingsStartup(Settings& settings, QWidget* parent = nullptr);
  ~SettingsStartup();

private:
  QString currentStartupDirectory() const;
  bool openFolder(const QString& dir) const;

private slots:
  void on_browse_plugins_clicked();
  void on_browse_scripts_clicked();
  void on_browse_sounds_clicked();
  void on_browse_worlds_clicked();
  void on_OpenAtStartup_add_clicked();
  void on_OpenAtStartup_down_clicked();
  void on_OpenAtStartup_itemSelectionChanged();
  void on_OpenAtStartup_remove_clicked();
  void on_OpenAtStartup_up_clicked();
  void on_StartupDirectory_browse_clicked();

private:
  Ui::SettingsStartup* ui;
  Settings& settings;
};
