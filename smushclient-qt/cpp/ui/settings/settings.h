#pragma once
#include "notifier.h"
#include <QtWidgets/QDialog>

namespace Ui {
class SettingsDialog;
} // namespace Ui

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
  ~SettingsDialog() override;

private:
  QWidget* paneForIndex(int row);

private slots:
  void on_settings_list_currentRowChanged(int row);

private:
  static SettingsNotifier notifier;

  Ui::SettingsDialog* ui;
  QWidget* pane = nullptr;
  Settings& settings;
};
