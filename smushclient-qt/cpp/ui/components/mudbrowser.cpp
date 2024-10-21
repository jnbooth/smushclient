#include "mudbrowser.h"
#include <QtGui/QMouseEvent>

// Public methods

MudBrowser::MudBrowser(QWidget *parent) : QTextBrowser(parent) {}

// Protected overrides

void MudBrowser::mouseMoveEvent(QMouseEvent *event)
{
  QTextBrowser::mouseMoveEvent(event);
  event->ignore();
}
