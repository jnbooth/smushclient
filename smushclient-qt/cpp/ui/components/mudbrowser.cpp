#include "mudbrowser.h"
#include "../../mudcursor.h"
#include "../../spans.h"
#include "mudscrollbar.h"
#include <QtGui/QAbstractTextDocumentLayout>
#include <QtGui/QMouseEvent>

// Public methods

MudBrowser::MudBrowser(QWidget* parent)
  : QTextBrowser(parent)
  , cursorPtr(new MudCursor(document()))
{
  setVerticalScrollBar(new MudScrollBar);
}

MudScrollBar*
MudBrowser::verticalScrollBar() const
{
  return qobject_cast<MudScrollBar*>(QAbstractScrollArea::verticalScrollBar());
}

MudCursor*
MudBrowser::cursor() const
{
  return cursorPtr;
}

// Public slots

void
MudBrowser::setKeypadIgnored(bool ignored)
{
  m_keypadIgnored = ignored;
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
  if (keypadIgnored() && event->modifiers().testFlag(
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
  const QPoint at = event->pos();
  const QTextCharFormat format = cursorForPosition(event->pos()).charFormat();
  const QString anchor = format.anchorHref();
  if (!anchor.isEmpty()) {
    const SendTo sendTo = spans::getSendTo(format);
    if (spans::hasPrompts(format)) {
      emit linkMenuActivated(at, anchor, spans::getPrompts(format), sendTo);
    } else {
      emit linkActivated(anchor, sendTo);
    }
  }
  QTextBrowser::mouseReleaseEvent(event);
}
