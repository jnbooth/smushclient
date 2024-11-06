#pragma once
#include <QtWidgets/QDialog>
#include <QtWidgets/QListWidgetItem>
#include "../settings.h"

namespace Ui
{
  class SettingsDialog;
}

class WorldTab;

class SettingsDialog : public QDialog
{
  Q_OBJECT

public:
  static void connect(WorldTab *tab);

  explicit SettingsDialog(Settings &settings, QWidget *parent = nullptr);
  ~SettingsDialog();

private:
  Ui::SettingsDialog *ui;
  QList<QWidget *> panes;
  qsizetype activePane;

  void setupPane(QWidget *pane, const char *key);

private slots:
  void on_settings_list_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
};
