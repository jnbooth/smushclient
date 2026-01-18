#pragma once
#include <QtWidgets/QDialog>

namespace Ui {
class SettingsDialog;
}

class MainWindow;
class Settings;
class WorldTab;

class SettingsDialog : public QDialog
{
  Q_OBJECT

public:
  static void connect(MainWindow* window);
  static void connect(WorldTab* tab);

  explicit SettingsDialog(Settings& settings, QWidget* parent = nullptr);
  ~SettingsDialog();

private:
  QWidget* paneForIndex(int row);

private slots:
  void on_settings_list_currentRowChanged(int row);

private:
  Ui::SettingsDialog* ui;
  QWidget* pane = nullptr;
  Settings& settings;
};
