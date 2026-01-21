#pragma once
#include <QtGui/QTextCursor>

struct Link;
enum class SendTo : uint8_t;
enum class TextStyle : uint16_t;

enum class LineType
{
  Output,
  Input,
  Note,
};

namespace spans {
QString
encodeLink(SendTo sendto, const QString& action);

SendTo
decodeLink(QString& link);

std::optional<SendTo>
getSendTo(const QTextCharFormat& format);

QFlags<TextStyle>
getStyles(const QTextCharFormat& format);

QString
getPrompts(const QTextCharFormat& format);

void
setLineType(QTextCharFormat& format, LineType type);

LineType
getLineType(const QTextCharFormat& format);

void
setTimestamp(QTextCursor& cursor);

QDateTime
getTimestamp(const QTextBlockFormat& format);

QString&
sanitizeHtml(QString& html);
} // namespace spans
