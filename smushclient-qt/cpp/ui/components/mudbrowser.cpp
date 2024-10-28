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

void MudBrowser::setIgnoreKeypad(bool ignore)
{
  ignoreKeypad = ignore;
}

// Protected overrides

void MudBrowser::keyPressEvent(QKeyEvent *event)
{
  if (ignoreKeypad && event->modifiers().testFlag(Qt::KeypadModifier)) [[unlikely]]
  {
    event->ignore();
    return;
  }
  QTextBrowser::keyPressEvent(event);
}

void MudBrowser::mouseMoveEvent(QMouseEvent *event)
{
  QTextBrowser::mouseMoveEvent(event);
  event->ignore();
}
