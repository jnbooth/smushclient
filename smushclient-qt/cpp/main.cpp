#include <QtGui/QFontDatabase>
#include <QtWidgets/QApplication>
#include "lua.h"
#include "environment.h"
#include "./ui/mainwindow.h"
#include "settings.h"

int main(int argc, char *argv[])
{
  setenv("LUA_PATH", SCRIPTS_DIR "/?.lua;" LUA_PATH_DEFAULT, false);
  QApplication app(argc, argv);
  QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/Dina"));
  QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/Dina-Bold"));
  QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/FixedSys"));
  Settings settings;
  MainWindow *w = new MainWindow;
  for (const QString &reopen : settings.lastFiles())
    w->openWorld(reopen);
  w->show();
  app.exec();
}
