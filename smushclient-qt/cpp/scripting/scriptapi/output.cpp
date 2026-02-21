#include "../../bytes.h"
#include "../../casting.h"
#include "../../layout.h"
#include "../../spans.h"
#include "../../ui/mudstatusbar/mudstatusbar.h"
#include "../../ui/ui_worldtab.h"
#include "../../ui/worldtab.h"
#include "../scriptapi.h"
#include "smushclient_qt/src/ffi/spans.cxx.h"
#include <QtGui/QClipboard>
#include <QtWidgets/QColorDialog>

using std::string_view;

// Public static methods

QColor
ScriptApi::GetSysColor(SysColor sysColor)
{
  const QPalette palette = QGuiApplication::palette();
  switch (sysColor) {
    case SysColor::Background:
      return palette.color(QPalette::ColorRole::Base);
    case SysColor::Window:
      return palette.color(QPalette::ColorRole::Window);
    case SysColor::WindowText:
      return palette.color(QPalette::ColorRole::WindowText);
    case SysColor::ButtonFace:
      return palette.color(QPalette::ColorRole::Button);
    case SysColor::ButtonShadow:
      return palette.color(QPalette::ColorRole::Shadow);
    case SysColor::ButtonText:
      return palette.color(QPalette::ColorRole::ButtonText);
    case SysColor::ButtonHighlight:
      return palette.color(QPalette::ColorRole::BrightText);
    case SysColor::Dark:
      return palette.color(QPalette::ColorRole::Dark);
    case SysColor::Light:
      return palette.color(QPalette::Light);
    default:
      return QColor();
  }
}

void
ScriptApi::SetClipboard(const QString& text)
{
  QGuiApplication::clipboard()->setText(text);
}

// Public methods

void
ScriptApi::AnsiNote(string_view text) const
{
  for (const StyledSpan& span : client.ansiNote(bytes::slice(text))) {
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

QColor
ScriptApi::PickColour(const QColor& hint) const
{
  return QColorDialog::getColor(hint, &tab);
}

QColor
ScriptApi::SetBackgroundColour(const QColor& color) const
{
  QWidget* background = tab.ui->background;
  QPalette palette = background->palette();
  const QColor oldColor = palette.color(QPalette::Base);
  if (color == Qt::GlobalColor::black) {
    palette.setColor(QPalette::Base, Qt::GlobalColor::transparent);
  } else if (color.isValid()) {
    palette.setColor(QPalette::Base, color);
  } else {
    palette.setColor(QPalette::Base, palette.color(QPalette::AlternateBase));
  }
  background->setPalette(palette);
  return oldColor == Qt::GlobalColor::transparent ? Qt::GlobalColor::black
                                                  : oldColor;
}

ApiCode
ScriptApi::SetCursor(Qt::CursorShape cursorShape) const
{
  tab.ui->area->setCursor(cursorShape);
  return ApiCode::OK;
}

QColor
ScriptApi::SetForegroundColour(const QColor& color) const
{
  QWidget* background = tab.ui->background;
  QPalette palette = background->palette();
  const QColor oldColor = palette.color(QPalette::Text);
  palette.setColor(QPalette::Text, color);
  palette.setColor(QPalette::HighlightedText, color);
  background->setPalette(palette);
  return oldColor;
}

QColor
ScriptApi::SetHighlightColour(const QColor& color) const
{
  QWidget* background = tab.ui->background;
  QPalette palette = background->palette();
  const QColor oldColor = palette.color(QPalette::Highlight);
  palette.setColor(QPalette::Highlight, color);
  background->setPalette(palette);
  return oldColor;
}

void
ScriptApi::SetStatus(const QString& status) const
{
  statusBar->setMessage(status);
}

void
ScriptApi::Simulate(string_view output) const
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
