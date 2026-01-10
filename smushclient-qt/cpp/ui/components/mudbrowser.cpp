#include "mudbrowser.h"
#include "mudscrollbar.h"

// Public methods

MudBrowser::MudBrowser(QWidget* parent)
  : QTextBrowser(parent)
{
  setVerticalScrollBar(new MudScrollBar);
}

MudScrollBar*
MudBrowser::verticalScrollBar() const
{
  return qobject_cast<MudScrollBar*>(QAbstractScrollArea::verticalScrollBar());
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
