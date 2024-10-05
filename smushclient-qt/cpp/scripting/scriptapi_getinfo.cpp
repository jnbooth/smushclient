#include "scriptapi.h"
#include <QtGui/QTextBlock>
#include <QtNetwork/QTcpSocket>
#include "../ui/worldtab.h"
#include "../ui/ui_worldtab.h"

using std::string;

// Private utils

inline QString convertString(const string &s)
{
  return QString::fromUtf8(s.data(), s.size());
}

// Public methods

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
    return QFontMetrics(font).leading();
  case 5: // external leading
    return 0;
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
    const QFont::StyleHint hint = font.styleHint();
    if (hint == QFont::StyleHint::Monospace)
      return FontPitchFlag::Monospace;

    const FontPitchFlag pitchFlag =
        QFontInfo(font).fixedPitch() ? FontPitchFlag::Fixed : FontPitchFlag::Variable;

    switch (hint)
    {
    case QFont::StyleHint::Serif:
      return FontFamilyFlag::Roman | pitchFlag;
    case QFont::StyleHint::SansSerif:
      return FontFamilyFlag::Swiss | pitchFlag;
    case QFont::StyleHint::TypeWriter:
      return FontFamilyFlag::Modern | pitchFlag;
    case QFont::StyleHint::Cursive:
      return FontFamilyFlag::Script | pitchFlag;
    case QFont::StyleHint::Decorative:
      return FontFamilyFlag::Decorative | pitchFlag;
    default:
      return FontFamilyFlag::AnyFamily | pitchFlag;
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
    return !socket->isOpen();
  case 239:
    return (int)actionSource;
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

QVariant ScriptApi::GetLineInfo(int line, int infoType) const
{
  const QTextBlock block = cursor.document()->findBlockByLineNumber(line);
  switch (infoType)
  {
  case 1:
    return block.text();
  case 2:
    return block.text().length();
  case 3:
    return block.text().isEmpty();
  default:
    return QVariant();
  }
}

QVariant MiniWindow::info(int infoType) const
{
  switch (infoType)
  {
  case 1:
    return location.x();
  case 2:
    return location.y();
  case 3:
    return width();
  case 4:
    return height();
  case 5:
    return isVisible();
  case 6:
    return isHidden();
  case 7:
    return (int)position;
  case 8:
    return (int)flags;
  case 9:
    return background;
  case 10:
    return rect().left();
  case 11:
    return rect().top();
  case 12:
    return rect().right();
  case 13:
    return rect().bottom();
  case 14:
    return mapFromGlobal(QCursor::pos()).x();
  case 15:
    return mapFromGlobal(QCursor::pos()).y();
  case 16:
    return parentWidget()->mapFromGlobal(QCursor::pos()).x();
  case 17:
    return parentWidget()->mapFromGlobal(QCursor::pos()).y();
  case 18:
    return 0;
  case 19:
    return installed;
  case 20:
    return zOrder;
  case 21:
    return pluginID;
  default:
    return QVariant();
  }
}

QVariant Hotspot::info(int infoType) const
{
  switch (infoType)
  {
  case 1:
    return rect().left();
  case 2:
    return rect().top();
  case 3:
    return rect().right();
  case 4:
    return rect().bottom();
  case 5:
    return convertString(callbacks.mouseOver);
  case 6:
    return convertString(callbacks.cancelMouseOver);
  case 7:
    return convertString(callbacks.mouseDown);
  case 8:
    return convertString(callbacks.cancelMouseDown);
  case 9:
    return convertString(callbacks.mouseUp);
  case 10:
    return toolTip();
  case 11:
    switch (cursor().shape())
    {
    case Qt::CursorShape::ArrowCursor:
      return 0;
    case Qt::CursorShape::BlankCursor:
      return -1;
    case Qt::CursorShape::OpenHandCursor:
      return 1;
    case Qt::CursorShape::IBeamCursor:
      return 2;
    case Qt::CursorShape::CrossCursor:
      return 3;
    case Qt::CursorShape::WaitCursor:
      return 4;
    case Qt::CursorShape::UpArrowCursor:
      return 5;
    case Qt::CursorShape::SizeFDiagCursor:
      return 6;
    case Qt::CursorShape::SizeBDiagCursor:
      return 7;
    case Qt::CursorShape::SizeHorCursor:
      return 8;
    case Qt::CursorShape::SizeVerCursor:
      return 9;
    case Qt::CursorShape::SizeAllCursor:
      return 10;
    case Qt::CursorShape::ForbiddenCursor:
      return 11;
    case Qt::WhatsThisCursor:
      return 12;
    default:
      return 0;
    }
  case 12:
    return (underMouse() ? 0x80 : 0) + (int)hasMouseTracking();
  case 13:
    return convertString(callbacks.dragMove);
  case 14:
    return convertString(callbacks.dragRelease);
  case 15: // DragHandler flags
    return 0;
  default:
    return QVariant();
  }
}
