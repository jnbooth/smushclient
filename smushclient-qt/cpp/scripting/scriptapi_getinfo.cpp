#include "scriptapi.h"
#include <QtNetwork/QTcpSocket>
#include "../ui/worldtab.h"
#include "../ui/ui_worldtab.h"

QVariant ScriptApi::FontInfo(const QFont &font, int infoType) const
{
  switch (infoType)
  {
  case 1:
    return QFontMetrics(font).height();
  case 2:
    return QFontMetrics(font).ascent();
  case 3:
    return QFontMetrics(font).descent();
  case 4: // internal leading
    return 0;
  case 5: // external leading
    return QFontMetrics(font).leading();
  case 6:
    return QFontMetrics(font).averageCharWidth();
  case 7:
    return QFontMetrics(font).maxWidth();
  case 8:
    return font.weight();
  case 9: // overhang
    return QFontMetrics(font).leftBearing(QChar::fromLatin1('l'));
  // case 10: digitized aspect X
  // case 11: digitized aspect Y
  // case 12: first character defined in font
  // case 13: last character defined in font
  // case 14: default character substituted for those not in font
  // case 15: character used to define word breaks
  case 16:
    return QFontInfo(font).italic();
  case 17:
    return QFontInfo(font).underline();
  case 18:
    return QFontInfo(font).strikeOut();
  case 19: // pitch and family
  {
    QFontInfo info(font);
    const int pitchFlag = info.fixedPitch() ? 2 : 1;
    switch (font.styleHint())
    {
    case QFont::StyleHint::Monospace:
      return 8;
    case QFont::StyleHint::Serif:
      return 16 | pitchFlag;
    case QFont::StyleHint::SansSerif:
      return 32 | pitchFlag;
    case QFont::StyleHint::TypeWriter:
      return 48 | pitchFlag;
    case QFont::StyleHint::Cursive:
      return 64 | pitchFlag;
    case QFont::StyleHint::Decorative:
      return 80 | pitchFlag;
    default:
      return pitchFlag;
    }
  }
  // case 20: character set
  case 21:
    return QFontInfo(font).family();
  default:
    return QVariant();
  }
}

QVariant ScriptApi::GetInfo(int infoType) const
{
  switch (infoType)
  {
  case 72:
    return QStringLiteral(SCRIPTING_VERSION);
  case 106:
    return !tab()->socket->isOpen();
  case 272:
    return tab()->ui->area->contentsMargins().left();
  case 273:
    return tab()->ui->area->contentsMargins().top();
  case 274:
    return tab()->ui->area->contentsMargins().right();
  case 275:
    return tab()->ui->area->contentsMargins().bottom();
  case 280:
    return tab()->ui->area->height();
  case 281:
    return tab()->ui->area->width();
  default:
    return QVariant();
  }
}
