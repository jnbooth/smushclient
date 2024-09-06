#ifndef PREFSMXP_H
#define PREFSMXP_H

#include <QtWidgets/QWidget>
#include "cxx-qt-gen/ffi.cxxqt.h"

namespace Ui
{
  class PrefsMxp;
}

class PrefsMxp : public QWidget
{
  Q_OBJECT

public:
  explicit PrefsMxp(const World &world, QWidget *parent = nullptr);
  ~PrefsMxp();

private:
  Ui::PrefsMxp *ui;
};

#endif // PREFSMXP_H
