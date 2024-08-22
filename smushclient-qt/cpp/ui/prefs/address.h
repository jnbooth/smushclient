#ifndef PREFSADDRESS_H
#define PREFSADDRESS_H

#include <QtWidgets/QWidget>
#include "abstractpane.h"

namespace Ui
{
  class PrefsAddress;
}

class PrefsAddress : public AbstractPrefsPane
{
  Q_OBJECT

public:
  explicit PrefsAddress(const World *world, QWidget *parent = nullptr);
  ~PrefsAddress();

private:
  Ui::PrefsAddress *ui;
};

#endif // PREFSADDRESS_H
