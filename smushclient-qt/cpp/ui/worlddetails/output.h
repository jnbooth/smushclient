#pragma once
#include "smushclient_qt/src/ffi/audio.cxxqt.h"
#include <QtWidgets/QWidget>

class ScriptApi;

namespace Ui {
class PrefsOutput;
} // namespace Ui

class World;

class PrefsOutput : public QWidget
{
  Q_OBJECT

public:
  explicit PrefsOutput(const World& world, QWidget* parent = nullptr);
  ~PrefsOutput() override;

private slots:
  void on_NewActivitySound_browse_clicked();
  void on_NewActivitySound_test_clicked();
  void on_NewActivitySound_textChanged(const QString& text);
  void on_reset_clicked();

private:
  Ui::PrefsOutput* ui;
  RustFilePlayback audio{};
};
