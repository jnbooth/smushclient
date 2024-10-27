#pragma once
#include <QtWidgets/QDialog>
#include <QtWidgets/QListWidgetItem>
#include "../settings.h"

namespace Ui
{
  class SettingsDialog;
}

class SettingsDialog : public QDialog
{
  Q_OBJECT

public:
  explicit SettingsDialog(Settings &settings, QWidget *parent = nullptr);
  ~SettingsDialog();

signals:
  void inputBackgroundChanged(const QColor &color);
  void inputForegroundChanged(const QColor &color);
  void inputFontChanged(const QFont &font);
  void outputFontChanged(const QFont &font);

private:
  Ui::SettingsDialog *ui;
  QList<QWidget *> panes;
  qsizetype activePane;

  void setupPane(QWidget *pane, const char *key);

private slots:
  void on_settings_list_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
};
