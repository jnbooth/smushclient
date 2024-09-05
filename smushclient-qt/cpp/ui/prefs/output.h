#ifndef PREFSOUTPUT_H
#define PREFSOUTPUT_H

#include <QtWidgets/QWidget>
#include "cxx-qt-gen/ffi.cxxqt.h"

namespace Ui
{
  class PrefsOutput;
}

class PrefsOutput : public QWidget
{
  Q_OBJECT

public:
  explicit PrefsOutput(World *world, QWidget *parent = nullptr);
  ~PrefsOutput();

private slots:
  void on_OutputFont_currentFontChanged(const QFont &f);

private:
  World *world;
  Ui::PrefsOutput *ui;
};

#endif // PREFSOUTPUT_H
