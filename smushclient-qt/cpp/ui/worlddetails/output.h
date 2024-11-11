#pragma once
#include <QtWidgets/QWidget>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QAudioOutput>
#include "cxx-qt-gen/ffi.cxxqt.h"

class ScriptApi;

namespace Ui
{
  class PrefsOutput;
}

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
};
