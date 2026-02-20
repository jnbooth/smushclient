#include "spans.h"
#include "smushclient_qt/src/ffi/spans.cxx.h"
#include <QtCore/QDateTime>
#include <QtCore/QRegularExpression>

namespace property {
const QTextCharFormat::Property styles =
  static_cast<QTextCharFormat::Property>(ffi::spans::SpanProperty::Styles);

const QTextCharFormat::Property sendTo =
  static_cast<QTextCharFormat::Property>(ffi::spans::SpanProperty::SendTo);

const QTextCharFormat::Property prompts =
  static_cast<QTextCharFormat::Property>(ffi::spans::SpanProperty::Prompts);

const QTextCharFormat::Property lineType =
  static_cast<QTextCharFormat::Property>(ffi::spans::SpanProperty::LineType);

const QTextBlockFormat::Property timestamp = QTextBlockFormat::UserProperty;
} // namespace property

// Private utils

namespace {
template<typename T>
T
getUnderlying(const QTextCharFormat& format, QTextCharFormat::Property prop)
{
  return static_cast<T>(
    format.property(prop).value<std::underlying_type_t<T>>());
}

template<typename T>
void
setUnderlying(QTextCharFormat& format, QTextCharFormat::Property prop, T value)
{
  format.setProperty(prop, static_cast<std::underlying_type_t<T>>(value));
}
} // namespace

// Public functions

namespace spans {
LineType
getLineType(const QTextCharFormat& format)
{
  return getUnderlying<LineType>(format, property::lineType);
}

QStringList
getPrompts(const QTextCharFormat& format)
{
  const QString prompts = format.property(property::prompts).toString();
  return prompts.isEmpty() ? QStringList() : prompts.split(QChar(0x1E));
}

SendTo
getSendTo(const QTextCharFormat& format)
{
  return getUnderlying<SendTo>(format, property::sendTo);
}

QFlags<TextStyle>
getStyles(const QTextCharFormat& format)
{
  return QFlags<TextStyle>::fromInt(format.property(property::styles).toInt());
}

QDateTime
getTimestamp(const QTextBlockFormat& format)
{
  return format.property(property::timestamp).toDateTime();
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
  cursor.setBlockFormat(format);
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
