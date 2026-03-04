#include "./ui/mainwindow.h"
#include "./ui/notepad/notepads.h"
#include "environment.h"
#include "settings.h"
#include <QtGui/QFontDatabase>
#include <QtWidgets/QApplication>

int
main(int argc, char* argv[])
{
#if defined(Q_OS_WINDOWS)
  QCoreApplication::setOrganizationName(QStringLiteral(CMAKE_ORG_NAME));
#endif
  QApplication app(argc, argv);
  QCoreApplication::setApplicationName(QStringLiteral(CMAKE_APP_NAME));
  QCoreApplication::setApplicationVersion(QStringLiteral(CMAKE_APP_VERSION));
  Settings settings;
  initializeStartupDirectory(settings.getStartupDirectoryOrDefault());
  Notepads* notepads = new Notepads;
  MainWindow* w = new MainWindow(*notepads);
  for (const QString& reopen : settings.getStartupWorlds()) {
    w->openWorld(reopen);
  }
  w->show();
  app.exec(); // NOLINT(readability-static-accessed-through-instance)
}
