#include "scriptapi.h"
#include <QtNetwork/QTcpSocket>
#include "../ui/worldtab.h"
#include "../ui/ui_worldtab.h"

QVariant ScriptApi::GetInfo(int infoType) const
{
  switch (infoType)
  {
  case 72:
    return QStringLiteral(SCRIPTING_VERSION);
  case 106:
    return !tab()->socket->isOpen();
  case 272:
    return tab()->ui->area->contentsMargins().left();
  case 273:
    return tab()->ui->area->contentsMargins().top();
  case 274:
    return tab()->ui->area->contentsMargins().right();
  case 275:
    return tab()->ui->area->contentsMargins().bottom();
  case 280:
    return tab()->ui->area->height();
  case 281:
    return tab()->ui->area->width();
  default:
    return QVariant();
  }
}
