#pragma once

#include <cxx-qt-lib/core/qlist/qlist_private.h>

namespace rust {
namespace smushclientqtlib1 {
namespace qlist {
template<typename T>
void
qlistClear(QList<T>& list)
{
  list.clear();
}

template<typename T>
bool
qlistContains(const QList<T>& list, const T& item)
{
  return list.contains(item);
}
} // namespace qlist

} // namespace smushclientqtlib1

} // namespace rust
