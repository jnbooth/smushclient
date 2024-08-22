#ifndef PREFSCUSTOMCOLOR_H
#define PREFSCUSTOMCOLOR_H

#include <QtWidgets/QWidget>
#include "abstractpane.h"

namespace Ui
{
  class PrefsCustomColor;
}

class PrefsCustomColor : public AbstractPrefsPane
{
  Q_OBJECT

public:
  explicit PrefsCustomColor(const World *world, QWidget *parent = nullptr);
  ~PrefsCustomColor();

private:
  Ui::PrefsCustomColor *ui;
};

#endif // PREFSCUSTOMCOLOR_H
