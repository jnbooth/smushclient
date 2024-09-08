#ifndef PREFSCOMMANDS_H
#define PREFSCOMMANDS_H

#include <QtWidgets/QWidget>
#include "cxx-qt-gen/ffi.cxxqt.h"

namespace Ui
{
  class PrefsCommands;
}

class PrefsCommands : public QWidget
{
  Q_OBJECT

public:
  explicit PrefsCommands(World &world, QWidget *parent = nullptr);
  ~PrefsCommands();

private slots:
  void on_InputFont_currentFontChanged(const QFont &f);

private:
  Ui::PrefsCommands *ui;
  World &world;
};

#endif // PREFSCOMMANDS_H
