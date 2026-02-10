#pragma once
#include <QtWidgets/QWidget>

namespace Ui {
class SettingsHistory;
} // namespace Ui

class Settings;
class SettingsNotifier;

class SettingsHistory : public QWidget
{
  Q_OBJECT

public:
  explicit SettingsHistory(Settings& settings,
                           SettingsNotifier& notifier,
                           QWidget* parent = nullptr);
  ~SettingsHistory() override;

private slots:
  void updateInputHistoryLimit();
  void updateOutputLimit();
  void updateOutputHistoryLimit();

private:
  Ui::SettingsHistory* ui;
  SettingsNotifier& notifier;
};
