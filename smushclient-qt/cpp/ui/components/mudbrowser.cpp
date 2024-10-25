#include "mudbrowser.h"
#include <QtGui/QMouseEvent>
#include "mudscrollbar.h"

// Public methods

MudBrowser::MudBrowser(QWidget *parent)
    : QTextBrowser(parent)
{
  setVerticalScrollBar(new MudScrollBar);
}

MudScrollBar *MudBrowser::verticalScrollBar() const
{
  return qobject_cast<MudScrollBar *>(QAbstractScrollArea::verticalScrollBar());
}

// Protected overrides

void MudBrowser::mouseMoveEvent(QMouseEvent *event)
{
  QTextBrowser::mouseMoveEvent(event);
  event->ignore();
}
