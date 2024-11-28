#pragma once
#include <QtMultimedia/QAudioOutput>
#include <QtMultimedia/QMediaPlayer>
#include <QtWidgets/QWidget>

namespace Ui
{
  class SettingsSound;
}

class Settings;

class SettingsSound : public QWidget
{
  Q_OBJECT

public:
  explicit SettingsSound(Settings &settings, QWidget *parent = nullptr);
  ~SettingsSound();

private:
  Ui::SettingsSound *ui;
  QAudioOutput audio{};
  QMediaPlayer player{};

private slots:
  void on_BellSound_browse_clicked();
  void on_BellSound_test_clicked();
  void on_BellSound_textChanged(const QString &text);
};
