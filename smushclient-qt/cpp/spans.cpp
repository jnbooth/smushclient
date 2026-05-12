#include "spans.h"
#include "casting.h"
#include <QtCore/QDateTime>
#include <QtCore/QRegularExpression>

namespace property {
const int styles = static_cast<int>(ffi::spans::SpanProperty::Styles);
const int sendTo = static_cast<int>(ffi::spans::SpanProperty::SendTo);
const int prompts = static_cast<int>(ffi::spans::SpanProperty::Prompts);
const int lineType = static_cast<int>(ffi::spans::SpanProperty::LineType);

const int timestamp = QTextBlockFormat::UserProperty;
const int timer = QTextBlockFormat::UserProperty + 1;
} // namespace property

// Public functions

namespace spans {
QElapsedTimer
getElapsed(const QTextBlockFormat& format)
{
  return qvariant_cast<QElapsedTimer>(format.property(property::timestamp));
}

LineType
getLineType(const QTextCharFormat& format)
{
  return qvariant_enum_cast<LineType>(format.property(property::lineType));
}

QString
getPrompts(const QTextCharFormat& format)
{
  return format.property(property::prompts).toString();
}

SendTo
getSendTo(const QTextCharFormat& format)
{
  return qvariant_enum_cast<SendTo>(format.property(property::sendTo));
}

TextStyles
getStyles(const QTextCharFormat& format)
{
  return TextStyles::fromInt(format.property(property::styles).toInt());
}

QDateTime
getTimestamp(const QTextBlockFormat& format)
{
  return format.property(property::timestamp).toDateTime();
}

bool
hasPrompts(const QTextCharFormat& format)
{
  return format.hasProperty(property::prompts);
}

void
setLineType(QTextCharFormat& format, LineType type)
{
  format.setProperty(property::lineType, to_underlying(type));
}

void
setSendTo(QTextCharFormat& format, SendTo sendTo)
{
  format.setProperty(property::sendTo, to_underlying(sendTo));
}

void
setTimestamp(QTextCursor& cursor)
{
  QTextBlockFormat format;
  format.setProperty(property::timestamp, QDateTime::currentDateTime());
  QElapsedTimer timer;
  timer.start();
  format.setProperty(property::timer, QVariant::fromValue(timer));
  cursor.setBlockFormat(format);
}

QString&
sanitizeHtml(QString& html)
{
  using Qt::StringLiterals::operator""_L1;

  static const QString none;
  static const QRegularExpression sanitize(
    R"(<!--.*?-->|"
    " ?(background-color:transparent|-qt-paragraph-type:empty|(\w|-)+:0(spx)?)"
    ";? ?)"_L1);
  static const QRegularExpression attributeWhitespace("=\"\\s+"_L1);
  static const QRegularExpression emptyAttribute(R"( ?(\w|-)+="")"_L1);

  const qsizetype bodyStart = html.indexOf("<body>"_L1) + 6;
  const qsizetype bodyEnd = html.lastIndexOf("</body>"_L1);

  return html.slice(bodyStart, bodyEnd - bodyStart)
    .replace(sanitize, none)
    .replace(attributeWhitespace, "=\""_L1)
    .replace(R"( ?\w+="")"_L1, none)
    .replace(emptyAttribute, none)
    .replace("href=\"w:"_L1, "href=\""_L1);
}
} // namespace spans
