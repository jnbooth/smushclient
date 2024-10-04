#include "link.h"
#include "cxx-qt-gen/ffi.cxxqt.h"

static const QString internetPrefix = QStringLiteral("i:");
static const QString worldPrefix = QStringLiteral("w:");
static const QString inputPrefix = QStringLiteral("n:");

// Public functions

constexpr const QString &getPrefix(SendTo sendto)
{
  switch (sendto)
  {
  case SendTo::Internet:
    return internetPrefix;
  case SendTo::World:
    return worldPrefix;
  case SendTo::Input:
    return inputPrefix;
  }
}

QString encodeLink(SendTo sendto, const QString &action)
{
  QString link = action;
  link.prepend(getPrefix(sendto));
  return link;
}

SendTo decodeLink(QString &link)
{
  switch (link.front().toLatin1())
  {
  case 'i':
    link.remove(0, 2);
    return SendTo::Internet;
  case 'n':
    link.remove(0, 2);
    return SendTo::Input;
  case 'w':
    link.remove(0, 2);
    return SendTo::World;
  default:
    return SendTo::Internet;
  }
}
