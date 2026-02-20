#include "mudbrowser.h"
#include "../../mudcursor.h"
#include "../../spans.h"
#include "mudscrollbar.h"
#include <QtGui/QAbstractTextDocumentLayout>
#include <QtGui/QMouseEvent>

// Public methods

MudBrowser::MudBrowser(QWidget* parent)
  : QTextBrowser(parent)
  , mudCursor(nullptr)
{
  setVerticalScrollBar(new MudScrollBar);
}

MudScrollBar*
MudBrowser::verticalScrollBar() const
{
  return qobject_cast<MudScrollBar*>(QAbstractScrollArea::verticalScrollBar());
}

MudCursor*
MudBrowser::cursor()
{
  if (mudCursor == nullptr) {
    mudCursor = new MudCursor(document());
  }
  return mudCursor;
}

// Public slots

void
MudBrowser::setIgnoreKeypad(bool ignore)
{
  ignoreKeypad = ignore;
}

void
MudBrowser::setMaximumBlockCount(int maximum)
{
  document()->setMaximumBlockCount(maximum);
}

// Protected overrides

void
MudBrowser::keyPressEvent(QKeyEvent* event)
{
  if (ignoreKeypad && event->modifiers().testFlag(
                        Qt::KeyboardModifier::KeypadModifier)) [[unlikely]] {
    event->ignore();
    return;
  }
  QTextBrowser::keyPressEvent(event);
}

void
MudBrowser::mouseMoveEvent(QMouseEvent* event)
{
  QTextBrowser::mouseMoveEvent(event);
  event->ignore();
}

void
MudBrowser::mousePressEvent(QMouseEvent* event)
{
  if (event->buttons() == Qt::MouseButton::LeftButton &&
      event->modifiers() == Qt::KeyboardModifier::ControlModifier) {
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    const QString word = tc.selectedText();
    emit aliasMenuRequested(word);
    return;
  }
  QTextBrowser::mousePressEvent(event);
}

void
MudBrowser::mouseReleaseEvent(QMouseEvent* event)
{
  const QTextCharFormat format = document()
                                   ->documentLayout()
                                   ->formatAt(mapToContents(event->pos()))
                                   .toCharFormat();
  const QString anchor = format.anchorHref();
  if (!anchor.isEmpty()) {
    emit linkActivated(anchor, spans::getSendTo(format));
  }
  QTextBrowser::mouseReleaseEvent(event);
}

// Private methods

QPoint
MudBrowser::mapToContents(const QPoint& point) const
{
  const QScrollBar* hbar = horizontalScrollBar();
  const QScrollBar* vbar = verticalScrollBar();
  const int horizontalOffset = hbar == nullptr ? 0
                               : isRightToLeft()
                                 ? hbar->maximum() - hbar->value()
                                 : hbar->value();
  const int verticalOffset = verticalScrollBar() == nullptr ? 0 : vbar->value();
  return QPoint(point.x() + horizontalOffset, point.y() + verticalOffset);
}
