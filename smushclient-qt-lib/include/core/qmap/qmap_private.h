#pragma once

#include <cxx-qt-lib/core/qmap/qmap_private.h>

namespace rust {
namespace smushclientqtlib1 {
namespace qmap {
template<typename K, typename V>
void
qmapClear(QMap<K, V>& map)
{
  map.clear();
}

template<typename K, typename V>
bool
qmapContains(const QMap<K, V>& map, const K& item)
{
  return map.contains(item);
}
} // namespace qmap

} // namespace smushclientqtlib1

} // namespace rust
