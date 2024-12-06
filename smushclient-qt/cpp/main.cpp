#include <QtGui/QFontDatabase>
#include <QtWidgets/QApplication>
#include "lua.h"
#include "environment.h"
#include "./ui/notepad.h"
#include "./ui/mainwindow.h"
#include "settings.h"

int main(int argc, char *argv[])
{
#if defined(Q_OS_WINDOWS)
  _putenv_s("LUA_PATH", SCRIPTS_DIR "/?.lua;" LUA_PATH_DEFAULT);
#else
  setenv("LUA_PATH", SCRIPTS_DIR "/?.lua;" LUA_PATH_DEFAULT, false);
#endif
  QApplication app(argc, argv);
  QCoreApplication::setApplicationName(QStringLiteral(CMAKE_APP_NAME));
  QCoreApplication::setApplicationVersion(QStringLiteral(CMAKE_APP_VERSION));
#if defined(Q_OS_WINDOWS)
  QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/Dina.fon"));
#else
  QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/Dina.ttf"));
  QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/Dina-Bold.ttf"));
  QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/FixedSys.ttf"));
#endif
  Settings settings;
  initializeStartupDirectory(settings.getStartupDirectoryOrDefault());
  Notepads *notepads = new Notepads;
  MainWindow *w = new MainWindow(notepads);
  for (const QString &reopen : settings.getStartupWorlds())
    w->openWorld(reopen);
  w->show();
  app.exec();
}
