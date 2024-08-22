#ifndef PREFSMXP_H
#define PREFSMXP_H

#include <QtWidgets/QWidget>
#include "abstractpane.h"

namespace Ui
{
  class PrefsMxp;
}

class PrefsMxp : public AbstractPrefsPane
{
  Q_OBJECT

public:
  explicit PrefsMxp(const World *world, QWidget *parent = nullptr);
  ~PrefsMxp();

private:
  Ui::PrefsMxp *ui;
};

#endif // PREFSMXP_H
