#ifndef PREFSLOGGING_H
#define PREFSLOGGING_H

#include <QtWidgets/QWidget>
#include "cxx-qt-gen/ffi.cxxqt.h"

namespace Ui
{
  class PrefsLogging;
}

class PrefsLogging : public QWidget
{
  Q_OBJECT

public:
  explicit PrefsLogging(World &world, QWidget *parent = nullptr);
  ~PrefsLogging();

private slots:
  void on_LogFilePostamble_textChanged();
  void on_LogFilePreamble_textChanged();
  void on_LogFormatChanged(LogFormat value);
  void on_LogModeChanged(LogMode value);

private:
  Ui::PrefsLogging *ui;
  World &world;
};

#endif // PREFSLOGGING_H
