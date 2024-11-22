#include <QtGui/QFontDatabase>
#include <QtWidgets/QApplication>
#include "lua.h"
#include "environment.h"
#include "./ui/notepad.h"
#include "./ui/mainwindow.h"
#include "settings.h"

int main(int argc, char *argv[])
{
  setenv("LUA_PATH", SCRIPTS_DIR "/?.lua;" LUA_PATH_DEFAULT, false);
  QApplication app(argc, argv);
  QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/Dina.ttf"));
  QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/Dina-Bold.ttf"));
  QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/FixedSys.ttf"));
  Settings settings;
  Notepads *notepads = new Notepads;
  MainWindow *w = new MainWindow(notepads);
  for (const QString &reopen : settings.getStartupWorlds())
    w->openWorld(reopen);
  w->show();
  app.exec();
}
