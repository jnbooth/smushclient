#pragma once
#include "abstractprefstree.h"

namespace Ui {
class PrefsTimers;
}

class TimerModel;
class World;

class PrefsTimers : public AbstractPrefsTree {
  Q_OBJECT

public:
  PrefsTimers(const World &world, TimerModel *model, QWidget *parent = nullptr);
  ~PrefsTimers();

protected:
  void enableSingleButtons(bool enabled) override;
  void enableMultiButtons(bool enabled) override;

private:
  Ui::PrefsTimers *ui;
};
