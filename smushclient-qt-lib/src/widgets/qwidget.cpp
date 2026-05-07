#include "smushclient-qt-lib/qwidget.h"

namespace rust {
namespace smushclientqtlib1 {
void
qwidgetFontMetrics(const QWidget& widget, QFontMetrics* uninit)
{
  new (uninit) QFontMetrics(widget.fontMetrics());
}
} // namespace smushclientqtlib1
} // namespace rust
