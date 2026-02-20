#pragma once
#include <QtGui/QTextCursor>

struct Link;
enum class SendTo : uint8_t;
enum class TextStyle : uint16_t;

enum class LineType : uint8_t
{
  Output,
  Input,
  Note,
};

namespace spans {
LineType
getLineType(const QTextCharFormat& format);

QStringList
getPrompts(const QTextCharFormat& format);

SendTo
getSendTo(const QTextCharFormat& format);

QFlags<TextStyle>
getStyles(const QTextCharFormat& format);

QDateTime
getTimestamp(const QTextBlockFormat& format);

void
setLineType(QTextCharFormat& format, LineType type);

void
setSendTo(QTextCharFormat& format, SendTo sendTo);

void
setTimestamp(QTextCursor& cursor);

QString&
sanitizeHtml(QString& html);
} // namespace spans
