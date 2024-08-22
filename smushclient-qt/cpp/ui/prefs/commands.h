#ifndef PREFSCOMMANDS_H
#define PREFSCOMMANDS_H

#include <QtWidgets/QWidget>
#include "abstractpane.h"

namespace Ui
{
  class PrefsCommands;
}

class PrefsCommands : public AbstractPrefsPane
{
  Q_OBJECT

public:
  explicit PrefsCommands(World *world, QWidget *parent = nullptr);
  ~PrefsCommands();

private slots:
  void on_InputFont_currentFontChanged(const QFont &f);

private:
  World *world;
  Ui::PrefsCommands *ui;
};

#endif // PREFSCOMMANDS_H
