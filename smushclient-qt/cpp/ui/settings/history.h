#pragma once
#include <QtWidgets/QWidget>
#include "../../settings.h"

namespace Ui
{
  class History;
}

class SettingsNotifier;

class SettingsHistory : public QWidget
{
  Q_OBJECT

public:
  explicit SettingsHistory(Settings &settings, SettingsNotifier *notifier, QWidget *parent = nullptr);
  ~SettingsHistory();

private:
  Ui::History *ui;
  SettingsNotifier *notifier;
  Settings &settings;

private slots:
  void updateInputHistoryLimit();
  void updateOutputLimit();
  void updateOutputHistoryLimit();
};
