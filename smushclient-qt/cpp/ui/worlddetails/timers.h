#pragma once
#include "abstractprefstree.h"
#include "cxx-qt-gen/ffi.cxxqt.h"

namespace Ui
{
  class PrefsTimers;
}

class TimerModel;

class PrefsTimers : public AbstractPrefsTree
{
  Q_OBJECT

public:
  explicit PrefsTimers(const World &world, SmushClient &client, Timekeeper *timekeeper, QWidget *parent = nullptr);
  ~PrefsTimers();

protected:
  void enableSingleButtons(bool enabled) override;
  void enableMultiButtons(bool enabled) override;

private:
  Ui::PrefsTimers *ui;
  TimerModel *model;
};
