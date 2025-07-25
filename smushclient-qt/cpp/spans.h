#pragma once
#include <QString>
#include <QtCore/QDateTime>
#include <QtGui/QTextCursor>
#include <QtGui/QTextFormat>
#include <cstdint>
#include <optional>

struct Link;
enum class SendTo : uint8_t;
enum class TextStyle : uint16_t;

enum class LineType {
  Output,
  Input,
  Note,
};

void applyLink(QTextCharFormat &format, const Link &link);

QString encodeLink(SendTo sendto, const QString &action);

SendTo decodeLink(QString &link);

std::optional<SendTo> getSendTo(const QTextCharFormat &format);

void setStyles(QTextCharFormat &format, QFlags<TextStyle> styles);

QFlags<TextStyle> getStyles(const QTextCharFormat &format);

QString getPrompts(const QTextCharFormat &format);

void setLineType(QTextCharFormat &format, LineType type);

LineType getLineType(const QTextCharFormat &format);

void setTimestamp(QTextCursor &cursor);

QDateTime getTimestamp(const QTextBlockFormat &format);

QString &sanitizeHtml(QString &html);
