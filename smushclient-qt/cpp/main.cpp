#include <QtWidgets/QApplication>
#include "lua.h"
#include "environment.h"
#include "./ui/app.h"

int main(int argc, char *argv[])
{
  setenv("LUA_PATH", SCRIPTS_DIR "/?.lua;" LUA_PATH_DEFAULT, false);
  QApplication app(argc, argv);
  App w;
  w.show();
  return app.exec();
}
