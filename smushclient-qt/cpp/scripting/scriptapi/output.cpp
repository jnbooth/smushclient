#include "../../casting.h"
#include "../../layout.h"
#include "../../ui/components/mudscrollbar.h"
#include "../../ui/mudstatusbar/mudstatusbar.h"
#include "../../ui/ui_worldtab.h"
#include "../../ui/worldtab.h"
#include "../scriptapi.h"
#include "smushclient_qt/src/ffi/util.cxx.h"
#include <QtGui/QDesktopServices>
#include <QtGui/QFontDatabase>
#include <QtGui/QTextDocumentFragment>
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

rust::String
ScriptApi::FixupHTML(string_view text) noexcept
{
  return ffi::util::fixup_html(text);
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

// Public methods

void
ScriptApi::ActivateClient() const
{
  activateWindow(&tab);
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

QStringList
ScriptApi::GetRecentLines(int count) const
{
  return selectRecentLines(count).selectedText().split(paragraphSeparator);
}

void
ScriptApi::Pause(bool pause) const
{
  scrollBar.setPaused(pause);
}

void
ScriptApi::Reset() const noexcept
{
  client.resetMxp();
}

void
ScriptApi::ResetStatusTime() const
{
  statusBar()->resetStatusTime();
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
ScriptApi::SetOutputFont(const QString& family, qreal pointSize) const
{
  tab.ui->output->setFontFamily(family);
  tab.ui->output->setFontPointSize(pointSize);
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
