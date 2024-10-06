#include "spans.h"
#include "cxx-qt-gen/ffi.cxxqt.h"

using std::nullopt;
using std::optional;

constexpr int stylesProp = QTextCharFormat::UserProperty;
constexpr int promptsProp = QTextCharFormat::UserProperty + 1;
constexpr int typeProp = QTextCharFormat::UserProperty + 2;

constexpr int timestampProp = QTextBlockFormat::UserProperty;

static const QString internetPrefix = QStringLiteral("i:");
static const QString worldPrefix = QStringLiteral("w:");
static const QString inputPrefix = QStringLiteral("n:");

// Private utils

constexpr const QString &getPrefix(SendTo sendto)
{
  switch (sendto)
  {
  case SendTo::Internet:
    return internetPrefix;
  case SendTo::World:
    return worldPrefix;
  case SendTo::Input:
    return inputPrefix;
  }
}

// Public functions

void applyLink(QTextCharFormat &format, const Link &link)
{
  format.setAnchor(true);
  format.setAnchorHref(encodeLink(link.sendto, link.action));
  if (!link.hint.isEmpty())
    format.setToolTip(link.hint);
  if (!link.prompts.isEmpty())
    format.setProperty(promptsProp, link.prompts);
}

QString encodeLink(SendTo sendto, const QString &action)
{
  QString link = action;
  link.prepend(getPrefix(sendto));
  return link;
}

SendTo decodeLink(QString &link)
{
  switch (link.front().toLatin1())
  {
  case 'i':
    link.remove(0, 2);
    return SendTo::Internet;
  case 'n':
    link.remove(0, 2);
    return SendTo::Input;
  case 'w':
    link.remove(0, 2);
    return SendTo::World;
  default:
    return SendTo::Internet;
  }
}

optional<SendTo> getSendTo(const QTextCharFormat &format)
{
  const QString href = format.anchorHref();
  if (href.isEmpty())
    return nullopt;
  switch (href.front().toLatin1())
  {
  case 'n':
    return SendTo::Input;
  case 'w':
    return SendTo::World;
  default:
    return SendTo::Internet;
  }
}

void setStyles(QTextCharFormat &format, QFlags<TextStyle> styles)
{
  format.setProperty(stylesProp, styles.toInt());

  if (styles.testFlag(TextStyle::Bold))
    format.setFontWeight(QFont::Weight::Bold);

  if (styles.testFlag(TextStyle::Italic))
    format.setFontItalic(true);

  if (styles.testFlag(TextStyle::Strikeout))
    format.setFontStrikeOut(true);

  if (styles.testFlag(TextStyle::Underline))
    format.setFontUnderline(true);
}

QFlags<TextStyle> getStyles(const QTextCharFormat &format)
{
  return QFlags<TextStyle>::fromInt(format.property(stylesProp).toUInt());
}

QString getPrompts(const QTextCharFormat &format)
{
  return format.property(promptsProp).toString();
}

void setLineType(QTextCharFormat &format, LineType type)
{
  format.setProperty(typeProp, (int)type);
}

LineType getLineType(const QTextCharFormat &format)
{
  return (LineType)format.property(typeProp).toInt();
}

void setTimestamp(QTextCursor &cursor)
{
  QTextBlockFormat format = cursor.blockFormat();
  format.setProperty(timestampProp, QDateTime::currentDateTime());
  cursor.setBlockFormat(format);
}

QDateTime getTimestamp(const QTextBlockFormat &format)
{
  return format.property(timestampProp).toDateTime();
}
