#include "../../client.h"
#include "../../spans.h"
#include "../scriptapi.h"

using std::string_view;

using Qt::StringLiterals::operator""_L1;

// Public methods

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

QTextCharFormat
ScriptApi::GetNoteStyle() const
{
  return cursor->charFormat();
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
  cursor->appendHtml("<hr/>"_L1);
}

void
ScriptApi::NoteStyle(const QTextCharFormat& format) const
{
  cursor->mergeCharFormat(format);
}

void
ScriptApi::Tell(const QString& text)
{
  cursor->appendTell(text);
}
