#ifndef PREFSLOGGING_H
#define PREFSLOGGING_H

#include <QtWidgets/QWidget>
#include "abstractpane.h"

namespace Ui
{
  class PrefsLogging;
}

class PrefsLogging : public AbstractPrefsPane
{
  Q_OBJECT

public:
  explicit PrefsLogging(World *world, QWidget *parent = nullptr);
  ~PrefsLogging();

private slots:
  void on_LogFormat_clicked(int id);
  void on_LogMode_clicked(int id);

  void on_LogFilePreamble_textChanged();

  void on_LogFilePostamble_textChanged();

private:
  World *world;
  Ui::PrefsLogging *ui;
};

#endif // PREFSLOGGING_H
