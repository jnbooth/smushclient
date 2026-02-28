#include "../../casting.h"
#include "../../layout.h"
#include "../../spans.h"
#include "../../ui/mudstatusbar/mudstatusbar.h"
#include "../../ui/ui_worldtab.h"
#include "../../ui/worldtab.h"
#include "../scriptapi.h"
#include "smushclient_qt/src/ffi/spans.cxx.h"
#include <QtGui/QClipboard>
#include <QtGui/QFontDatabase>
#include <QtGui/QTextBlock>
#include <QtWidgets/QColorDialog>

using std::string_view;

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
  if (count < 0) {
    return;
  }
  QTextCursor eraseCursor(cursor->document());
  eraseCursor.movePosition(QTextCursor::MoveOperation::StartOfBlock,
                           QTextCursor::MoveMode::KeepAnchor);
  if (eraseCursor.hasSelection()) {
    --count;
  }
  eraseCursor.movePosition(QTextCursor::MoveOperation::PreviousBlock,
                           QTextCursor::MoveMode::KeepAnchor,
                           count);
  eraseCursor.removeSelectedText();
}

void
ScriptApi::DeleteOutput() const
{
  cursor->document()->clear();
}

int
ScriptApi::GetLinesInBufferCount() const
{
  return cursor->document()->lineCount();
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

ApiCode
ScriptApi::SetCursor(Qt::CursorShape cursorShape) const
{
  tab.ui->area->setCursor(cursorShape);
  return ApiCode::OK;
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
