#pragma once
#include <QtMultimedia/QAudioOutput>
#include <QtMultimedia/QMediaPlayer>
#include <QtWidgets/QWidget>

class ScriptApi;

namespace Ui
{
  class PrefsOutput;
}

class World;

class PrefsOutput : public QWidget
{
  Q_OBJECT

public:
  explicit PrefsOutput(const World &world, QWidget *parent = nullptr);
  ~PrefsOutput();

private:
  Ui::PrefsOutput *ui;
  QAudioOutput audio;
  QMediaPlayer player;

private slots:
  void on_NewActivitySound_browse_clicked();
  void on_NewActivitySound_test_clicked();
  void on_NewActivitySound_textChanged(const QString &text);
  void on_reset_clicked();
};
