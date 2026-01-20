#include "./ui/mainwindow.h"
#include "./ui/notepad.h"
#include "environment.h"
#include "settings.h"
#include <QtGui/QFontDatabase>
#include <QtWidgets/QApplication>
extern "C"
{
#include "lua.h"
}

int
main(int argc, char* argv[])
{
#if defined(Q_OS_WINDOWS)
  _putenv_s("LUA_PATH", SCRIPTS_DIR "/?.lua;" LUA_PATH_DEFAULT);
  QCoreApplication::setOrganizationName(QStringLiteral(CMAKE_ORG_NAME));
#else
  setenv("LUA_PATH", SCRIPTS_DIR "/?.lua;" LUA_PATH_DEFAULT, 0);
#endif
  QApplication app(argc, argv);
  QCoreApplication::setApplicationName(QStringLiteral(CMAKE_APP_NAME));
  QCoreApplication::setApplicationVersion(QStringLiteral(CMAKE_APP_VERSION));
  Settings settings;
  initializeStartupDirectory(settings.getStartupDirectoryOrDefault());
  Notepads* notepads = new Notepads;
  MainWindow* w = new MainWindow(notepads);
  for (const QString& reopen : settings.getStartupWorlds()) {
    w->openWorld(reopen);
  }
  w->show();
  app.exec(); // NOLINT(readability-static-accessed-through-instance)
}
