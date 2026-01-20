#pragma once
#include "smushclient_qt/src/ffi/audio.cxxqt.h"
#include <QtWidgets/QWidget>

namespace Ui {
class SettingsSound;
} // namespace Ui

class Settings;

class SettingsSound : public QWidget
{
  Q_OBJECT

public:
  explicit SettingsSound(Settings& settings, QWidget* parent = nullptr);
  ~SettingsSound() override;

private slots:
  void on_BellSound_browse_clicked();
  void on_BellSound_test_clicked();
  void on_BellSound_textChanged(const QString& text);

private:
  Ui::SettingsSound* ui;
  RustFilePlayback audio{};
};
