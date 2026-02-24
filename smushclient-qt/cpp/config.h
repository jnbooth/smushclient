#pragma once
#include <QtCore/QString>

namespace config {
inline QString
apiGuide()
{
  return QStringLiteral(
    "https://www.gammon.com.au/scripts/doc.php?general=function_list");
}

inline QString
issues()
{
  return QStringLiteral("https://github.com/jnbooth/smushclient/issues");
}
} // namespace config
