#pragma once
#include <QtWidgets/QWidget>

namespace Ui
{
  class SettingsHistory;
}

class Settings;
class SettingsNotifier;

class SettingsHistory : public QWidget
{
  Q_OBJECT

public:
  explicit SettingsHistory(Settings &settings, SettingsNotifier *notifier, QWidget *parent = nullptr);
  ~SettingsHistory();

private:
  Ui::SettingsHistory *ui;
  SettingsNotifier *notifier;

private slots:
  void updateInputHistoryLimit();
  void updateOutputLimit();
  void updateOutputHistoryLimit();
};
