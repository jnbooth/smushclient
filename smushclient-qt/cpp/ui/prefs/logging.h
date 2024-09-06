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
  void on_LogFormatIdClicked(int id);
  void on_LogModeIdClicked(int id);

private:
  World &world;
  Ui::PrefsLogging *ui;
};

#endif // PREFSLOGGING_H
