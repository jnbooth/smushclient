#ifndef PREFSCONNECTING_H
#define PREFSCONNECTING_H

#include <QtWidgets/QWidget>
#include "abstractpane.h"

namespace Ui
{
  class PrefsConnecting;
}

class PrefsConnecting : public AbstractPrefsPane
{
  Q_OBJECT

public:
  explicit PrefsConnecting(World *world, QWidget *parent = nullptr);
  ~PrefsConnecting();

private slots:
  void on_ConnectText_textChanged();

private:
  Ui::PrefsConnecting *ui;
  World *world;
};

#endif // PREFSCONNECTING_H
