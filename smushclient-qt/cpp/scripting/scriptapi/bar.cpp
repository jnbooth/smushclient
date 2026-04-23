#include "../../ui/mudstatusbar/mudstatusbar.h"
#include "../scriptapi.h"
#include <QtWidgets/QStatusBar>

void
ScriptApi::Info(const QString& text)
{
  infoCursor.insertText(text);
}

void
ScriptApi::InfoBackground(const QColor& color)
{
  QTextBlockFormat format;
  format.setBackground(color);
  infoCursor.select(QTextCursor::SelectionType::Document);
  infoCursor.mergeBlockFormat(format);
  infoCursor.clearSelection();
}

void
ScriptApi::InfoClear()
{
  infoCursor.select(QTextCursor::SelectionType::Document);
  infoCursor.removeSelectedText();
  infoCursor.clearSelection();
}

void
ScriptApi::InfoColour(const QColor& color)
{
  QTextCharFormat format;
  format.setForeground(color);
  infoCursor.mergeCharFormat(format);
}

void
ScriptApi::InfoFont(const QFont& font)
{
  QTextCharFormat format;
  format.setFont(font);
  infoCursor.mergeCharFormat(format);
}

void
ScriptApi::ResetStatusTime() const
{
  statusBar()->resetStatusTime();
}

void
ScriptApi::SetStatus(const QString& status) const
{
  statusBar()->setMessage(status);
}

void
ScriptApi::ShowInfoBar(bool visible)
{
  emit infoBarShown(visible);
}
