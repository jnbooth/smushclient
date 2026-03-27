#include "./ui/mainwindow.h"
#include "./ui/notepad/notepads.h"
#include "environment.h"
#include "settings.h"
#include "smushclient_qt/src/ffi/util.cxx.h"
#include <QtGui/QFontDatabase>
#include <QtWidgets/QApplication>

namespace {
void
handleMessage(QtMsgType msgtype,
              const QMessageLogContext& context,
              const QString& message)
{
  ffi::util::log(msgtype,
                 message,
                 context.category,
                 context.file,
                 context.function,
                 context.line);
}
} // namespace

int
main(int argc, char* argv[])
{
  qInstallMessageHandler(handleMessage);
  ffi::util::init_logger();
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
