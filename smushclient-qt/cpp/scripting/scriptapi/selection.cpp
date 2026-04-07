#include "../../ui/ui_worldtab.h"
#include "../../ui/worldtab.h"
#include "../scriptapi.h"
#include <QtGui/QClipboard>
#include <QtGui/QTextBlock>

using std::string_view;

// Public static methods

QString
ScriptApi::GetClipboard()
{
  return QGuiApplication::clipboard()->text();
}

void
ScriptApi::SetClipboard(const QString& text)
{
  QGuiApplication::clipboard()->setText(text);
}

// Public methods

QString
ScriptApi::GetSelection() const
{
  return tab.ui->output->textCursor().selectedText().replace(
    u'\n', QChar::ParagraphSeparator);
}

int
ScriptApi::GetSelectionEndColumn() const
{
  QTextCursor selectCursor = tab.ui->output->textCursor();
  if (!selectCursor.hasSelection()) {
    return -1;
  }
  selectCursor.setPosition(selectCursor.selectionEnd());
  return selectCursor.positionInBlock();
}

int
ScriptApi::GetSelectionEndLine() const
{
  QTextCursor selectCursor = tab.ui->output->textCursor();
  if (!selectCursor.hasSelection()) {
    return -1;
  }
  selectCursor.setPosition(selectCursor.selectionEnd());
  return selectCursor.blockNumber();
}

int
ScriptApi::GetSelectionStartColumn() const
{
  QTextCursor selectCursor = tab.ui->output->textCursor();
  if (!selectCursor.hasSelection()) {
    return -1;
  }
  selectCursor.setPosition(selectCursor.selectionStart());
  return selectCursor.positionInBlock();
}

int
ScriptApi::GetSelectionStartLine() const
{
  QTextCursor selectCursor = tab.ui->output->textCursor();
  if (!selectCursor.hasSelection()) {
    return -1;
  }
  selectCursor.setPosition(selectCursor.selectionStart());
  return selectCursor.blockNumber();
}

void
ScriptApi::SetSelection(int startLine,
                        int endLine,
                        int startColumn,
                        int endColumn) const
{
  using MoveMode = QTextCursor::MoveMode;
  using MoveOperation = QTextCursor::MoveOperation;

  if (endLine < startLine ||
      (endLine == startLine && endColumn < startColumn) || startLine < 0 ||
      endLine < 0 || startColumn < 0 || endColumn < 0) {
    return;
  }
  QTextCursor selectCursor = tab.ui->output->textCursor();
  const QTextBlock block = cursor->document()->findBlockByNumber(startLine);
  if (!block.isValid()) {
    return;
  }
  selectCursor.setPosition(block.position());
  if (startColumn >= block.length()) {
    selectCursor.movePosition(MoveOperation::EndOfLine, MoveMode::MoveAnchor);
  } else {
    selectCursor.movePosition(
      MoveOperation::Right, MoveMode::MoveAnchor, startColumn);
  }
  selectCursor.movePosition(
    MoveOperation::NextBlock, MoveMode::KeepAnchor, endLine - startLine);
  if (endColumn >= selectCursor.block().length()) {
    selectCursor.movePosition(MoveOperation::EndOfLine, MoveMode::MoveAnchor);
  } else {
    selectCursor.movePosition(MoveOperation::NextCharacter,
                              MoveMode::KeepAnchor,
                              endLine == startLine ? endColumn - startColumn
                                                   : endColumn);
  }
  tab.ui->output->setTextCursor(selectCursor);
}
