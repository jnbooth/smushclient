#ifndef PREFSTRIGGERS_H
#define PREFSTRIGGERS_H

#include <QtWidgets/QWidget>
#include "abstractpane.h"

namespace Ui
{
  class PrefsTriggers;
}

class PrefsTriggers : public AbstractPrefsPane
{
  Q_OBJECT

public:
  explicit PrefsTriggers(const World *world, QWidget *parent = nullptr);
  ~PrefsTriggers();

private:
  Ui::PrefsTriggers *ui;
};

#endif // PREFSTRIGGERS_H
