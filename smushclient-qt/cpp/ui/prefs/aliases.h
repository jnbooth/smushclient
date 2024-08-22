#ifndef PREFSALIASES_H
#define PREFSALIASES_H

#include <QtWidgets/QWidget>
#include "abstractpane.h"

namespace Ui
{
  class PrefsAliases;
}

class PrefsAliases : public AbstractPrefsPane
{
  Q_OBJECT

public:
  explicit PrefsAliases(const World *world, QWidget *parent = nullptr);
  ~PrefsAliases();

private:
  Ui::PrefsAliases *ui;
};

#endif // PREFSALIASES_H
