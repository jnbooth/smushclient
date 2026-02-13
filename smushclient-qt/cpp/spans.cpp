#include "spans.h"
#include "smushclient_qt/src/ffi/spans.cxx.h"
#include <QtCore/QDateTime>
#include <QtCore/QRegularExpression>

using std::nullopt;
using std::optional;

constexpr int typeProp = QTextCharFormat::UserProperty + 10;

constexpr int timestampProp = QTextBlockFormat::UserProperty;

// Public functions

namespace spans {

QString
encodeLink(SendTo sendto, const QString& action)
{
  QString link = action;
  ffi::spans::encode_link(sendto, link);
  return link;
}

SendTo
decodeLink(QString& link)
{
  return ffi::spans::get_send_to(link);
}

optional<SendTo>
getSendTo(const QTextCharFormat& format)
{
  QString href = format.anchorHref();
  if (href.isEmpty()) {
    return nullopt;
  }
  return decodeLink(href);
}

QFlags<TextStyle>
getStyles(const QTextCharFormat& format)
{
  return QFlags<TextStyle>::fromInt(ffi::spans::get_styles(format));
}

QStringList
getPrompts(const QTextCharFormat& format)
{
  return ffi::spans::get_prompts(format);
}

void
setLineType(QTextCharFormat& format, LineType type)
{
  format.setProperty(typeProp, static_cast<int>(type));
}

LineType
getLineType(const QTextCharFormat& format)
{
  return static_cast<LineType>(format.property(typeProp).toInt());
}

void
setTimestamp(QTextCursor& cursor)
{
  QTextBlockFormat format;
  format.setProperty(timestampProp, QDateTime::currentDateTime());
  cursor.setBlockFormat(format);
}

QDateTime
getTimestamp(const QTextBlockFormat& format)
{
  return format.property(timestampProp).toDateTime();
}

QString&
sanitizeHtml(QString& html)
{
#define Q QStringLiteral

  static const QString none;
  static const QRegularExpression sanitize(
    Q("("
      " ?(background-color:transparent|-qt-paragraph-type:empty|(\\w|-)+:0("
      "px)?);? ?"
      "|"
      "<!--.*?-->"
      ")"));
  static const QRegularExpression attributeWhitespace(Q("=\"\\s+"));
  static const QRegularExpression emptyAttribute(Q(" ?(\\w|-)+=\"\""));

  const qsizetype bodyStart = html.indexOf(Q("<body>")) + 6;
  const qsizetype bodyEnd = html.lastIndexOf(Q("</body>"));

  return html.slice(bodyStart, bodyEnd - bodyStart)
    .replace(sanitize, none)
    .replace(attributeWhitespace, Q("=\""))
    .replace(Q(" ?\\w+=\"\""), none)
    .replace(emptyAttribute, none)
    .replace(Q("href=\"w:"), Q("href=\""));

#undef Q
}
} // namespace spans
