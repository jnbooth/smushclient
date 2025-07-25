#pragma once
#include "smushclient_qt/src/ffi/audio.cxxqt.h"
#include <QtWidgets/QWidget>

namespace Ui {
class SettingsSound;
}

class Settings;

class SettingsSound : public QWidget {
  Q_OBJECT

public:
  explicit SettingsSound(Settings &settings, QWidget *parent = nullptr);
  ~SettingsSound();

private slots:
  void on_BellSound_browse_clicked();
  void on_BellSound_test_clicked();
  void on_BellSound_textChanged(const QString &text);

private:
  Ui::SettingsSound *ui;
  RustPlayback audio{};
};
