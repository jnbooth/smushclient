#pragma once
#include <QtWidgets/QWidget>
#include "smushclient_qt/src/ffi/audio.cxxqt.h"

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

private slots:
  void on_NewActivitySound_browse_clicked();
  void on_NewActivitySound_test_clicked();
  void on_NewActivitySound_textChanged(const QString &text);
  void on_reset_clicked();

private:
  Ui::PrefsOutput *ui;
  RustPlayback audio{};
};
