#ifndef PREFSTIMERS_H
#define PREFSTIMERS_H

#include <QtWidgets/QWidget>
#include "abstractpane.h"

namespace Ui
{
  class PrefsTimers;
}

class PrefsTimers : public AbstractPrefsPane
{
  Q_OBJECT

public:
  explicit PrefsTimers(const World *world, QWidget *parent = nullptr);
  ~PrefsTimers();

private:
  Ui::PrefsTimers *ui;
};

#endif // PREFSTIMERS_H
