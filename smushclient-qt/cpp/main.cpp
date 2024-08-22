#include "./ui/app.h"

#include <QtWidgets/QApplication>
#include <QtNetwork/QTcpSocket>
#include <QtCore/qmetatype.h>

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  App w;
  w.show();
  return app.exec();
}
