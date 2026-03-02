#include "../../casting.h"
#include "../../layout.h"
#include "../../spans.h"
#include "../../ui/mudstatusbar/mudstatusbar.h"
#include "../../ui/ui_worldtab.h"
#include "../../ui/worldtab.h"
#include "../scriptapi.h"
#include "smushclient_qt/src/ffi/spans.cxx.h"
#include "smushclient_qt/src/ffi/util.cxx.h"
#include <QtGui/QClipboard>
#include <QtGui/QDesktopServices>
#include <QtGui/QFontDatabase>
#include <QtGui/QTextBlock>
#include <QtGui/QTextDocumentFragment>
#include <QtWidgets/QColorDialog>

using std::string_view;

constexpr QChar paragraphSeparator(static_cast<char32_t>(2029));

// Public static methods

ApiCode
ScriptApi::AddFont(const QString& fileName)
{
  if (fileName.isEmpty()) {
    return ApiCode::BadParameter;
  }
  return QFontDatabase::addApplicationFont(fileName) == -1
           ? ApiCode::FileNotFound
           : ApiCode::OK;
}

rust::String
ScriptApi::FixupHTML(string_view text) noexcept
{
  return ffi::util::fixup_html(text);
}

QString
ScriptApi::GetClipboard()
{
  return QGuiApplication::clipboard()->text();
}

ApiCode
ScriptApi::OpenBrowser(const QString& url)
{
  const QUrl parsedUrl(url);
  if (!parsedUrl.isValid()) {
    return ApiCode::BadParameter;
  }
  return QDesktopServices::openUrl(parsedUrl) ? ApiCode::OK
                                              : ApiCode::CouldNotOpenFile;
}

void
ScriptApi::SetClipboard(const QString& text)
{
  QGuiApplication::clipboard()->setText(text);
}

// Public methods

void
ScriptApi::ActivateClient() const
{
  activateWindow(&tab);
}

void
ScriptApi::AnsiNote(string_view text) const
{
  for (const StyledSpan& span : client.ansiNote(text)) {
    cursor->appendText(span.text, span.format);
  }
}

void
ScriptApi::ColourTell(const QColor& foreground,
                      const QColor& background,
                      const QString& text)
{
  QTextCharFormat format;
  if (foreground.isValid()) {
    format.setForeground(foreground);
  }
  if (background.isValid()) {
    format.setBackground(background);
  }
  cursor->appendTell(text, format);
}

void
ScriptApi::DeleteLines(int count) const
{
  selectRecentLines(count).removeSelectedText();
}

void
ScriptApi::DeleteOutput() const
{
  cursor->document()->clear();
}

int
ScriptApi::GetLinesInBufferCount() const
{
  return cursor->document()->blockCount();
}

QTextCharFormat
ScriptApi::GetNoteStyle() const
{
  return cursor->charFormat();
}

QStringList
ScriptApi::GetRecentLines(int count) const
{
  return selectRecentLines(count).selectedText().split(paragraphSeparator);
}

QString
ScriptApi::GetSelection() const
{
  return tab.ui->output->textCursor().selectedText().replace(
    u'\n', paragraphSeparator);
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
ScriptApi::Hyperlink(const QString& action,
                     const QString& text,
                     const QString& hint,
                     const QColor& foreground,
                     const QColor& background,
                     bool url,
                     bool noUnderline)
{
  QTextCharFormat format;
  format.setAnchorHref(action);
  spans::setSendTo(format, url ? SendTo::Internet : SendTo::World);
  format.setToolTip(hint.isEmpty() ? action : hint);
  if (foreground.isValid()) {
    format.setForeground(foreground);
  }
  if (background.isValid()) {
    format.setBackground(background);
  }
  if (!noUnderline) {
    format.setAnchor(true);
  }
  cursor->appendTell(text, format);
}

void
ScriptApi::NoteHr() const
{
  cursor->appendHtml(QStringLiteral("<hr/>"));
}

void
ScriptApi::NoteStyle(const QTextCharFormat& format) const
{
  cursor->mergeCharFormat(format);
}

ApiCode
ScriptApi::SetCursor(Qt::CursorShape cursorShape) const
{
  tab.ui->area->setCursor(cursorShape);
  return ApiCode::OK;
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

void
ScriptApi::SetMainTitle(const QString& title) const
{
  tab.setWindowTitle(title);
}

void
ScriptApi::SetStatus(const QString& status) const
{
  statusBar()->setMessage(status);
}

void
ScriptApi::SetTitle(const QString& title) const
{
  tab.setTitle(title);
}

void
ScriptApi::Simulate(string_view output) const noexcept
{
  tab.simulateOutput(output);
}

void
ScriptApi::Tell(const QString& text)
{
  cursor->appendTell(text);
}

ApiCode
ScriptApi::TextRectangle(const QMargins& margins,
                         int borderOffset,
                         const QColor& borderColor,
                         int borderWidth,
                         const QBrush& outsideFill) const
{
  Ui::WorldTab& ui = *tab.ui;
  QTextDocument& doc = *ui.output->document();
  doc.setLayoutEnabled(false);
  QPalette palette;

  palette.setBrush(QPalette::ColorRole::Window, outsideFill);
  ui.area->setPalette(palette);
  ui.area->setContentsMargins(margins);

  palette.setBrush(QPalette::ColorRole::Window, borderColor);
  ui.outputBorder->setPalette(palette);
  ui.outputBorder->setContentsMargins(
    borderWidth, borderWidth, borderWidth, borderWidth);

  ui.background->setContentsMargins(
    borderOffset, borderOffset, borderOffset, borderOffset);
  doc.setLayoutEnabled(true);
  return ApiCode::OK;
}

ApiCode
ScriptApi::TextRectangle(const OutputLayout& layout) const
{
  return TextRectangle(layout.margins,
                       layout.borderOffset,
                       layout.borderColor,
                       layout.borderWidth,
                       layout.outsideFill);
}

ApiCode
ScriptApi::TextRectangle(const QRect& rect,
                         int borderOffset,
                         const QColor& borderColor,
                         int borderWidth,
                         const QBrush& outsideFill)
{
  assignedTextRectangle = rect;
  const QSize size = tab.ui->area->size();
  const QMargins margins(
    rect.left(),
    rect.top(),
    rect.right() > 0 ? size.width() - rect.right() : -rect.right(),
    rect.bottom() > 0 ? size.height() - rect.bottom() : -rect.bottom());
  const OutputLayout layout{
    .margins = margins,
    .borderOffset = clamped_cast<int16_t>(borderOffset),
    .borderColor = borderColor,
    .borderWidth = clamped_cast<int16_t>(borderWidth),
    .outsideFill = outsideFill,
  };
  client.setMetavariable("output/layout", layout.save());
  return TextRectangle(layout);
}

ApiCode
ScriptApi::TextRectangle() const
{
  const QByteArrayView variable = client.getMetavariable("output/layout");
  if (variable.isNull()) {
    return ApiCode::OK;
  }

  OutputLayout layout;
  if (!layout.restore(QByteArray(variable))) {
    return ApiCode::VariableNotFound;
  }

  return TextRectangle(layout);
}
