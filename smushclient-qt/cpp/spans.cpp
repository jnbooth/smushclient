#include "spans.h"
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

// Private utils

namespace {
template<typename T>
T
getUnderlying(const QTextCharFormat& format, int prop)
{
  return static_cast<T>(
    format.property(prop).value<std::underlying_type_t<T>>());
}

template<typename T>
void
setUnderlying(QTextCharFormat& format, int prop, T value)
{
  format.setProperty(prop, static_cast<std::underlying_type_t<T>>(value));
}
} // namespace

// Public functions

namespace spans {
QElapsedTimer
getElapsed(const QTextBlockFormat& format)
{
  return format.property(property::timestamp).value<QElapsedTimer>();
}

LineType
getLineType(const QTextCharFormat& format)
{
  return getUnderlying<LineType>(format, property::lineType);
}

QString
getPrompts(const QTextCharFormat& format)
{
  return format.property(property::prompts).toString();
}

SendTo
getSendTo(const QTextCharFormat& format)
{
  return getUnderlying<SendTo>(format, property::sendTo);
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
  setUnderlying(format, property::lineType, type);
}

void
setSendTo(QTextCharFormat& format, SendTo sendTo)
{
  setUnderlying(format, property::sendTo, sendTo);
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
