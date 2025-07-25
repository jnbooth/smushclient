#include "../../spans.h"
#include "../ui/ui_worldtab.h"
#include "../ui/worldtab.h"
#include "scriptapi.h"
#include "smushclient_qt/src/ffi/document.cxxqt.h"
#include <QtGui/QTextBlock>
#include <QtGui/QTextDocumentFragment>
#include <QtGui/QTextLayout>
#include <QtGui/QTextLine>

using std::optional;
using std::string;
using std::string_view;

// Private utils

inline QString convertString(const string &s) {
  return QString::fromUtf8(s.data(), s.size());
}

// Public methods

QVariant ScriptApi::FontInfo(const QFont &font, int infoType) const {
  switch (infoType) {
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
    return QFontMetrics(font).leftBearing(u'l');
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

    const int pitchFlag = QFontInfo(font).fixedPitch()
                              ? FontPitchFlag::Fixed
                              : FontPitchFlag::Variable;

    switch (hint) {
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

QVariant ScriptApi::GetInfo(int infoType) const {
  switch (infoType) {
  case 72:
    return QStringLiteral(SCRIPTING_VERSION);
  case 106:
    return !socket->isOpen();
  case 239:
    return (int)actionSource;
  case 240:
    return QFontMetrics(tab()->ui->output->font()).averageCharWidth();
  case 268:
#if defined(Q_OS_WIN)
    return (int)OperatingSystem::Windows;
#elif defined(Q_OS_MACOS)
    return (int)OperatingSystem::MacOS;
#elif defined(Q_OS_LINUX)
    return (int)OperatingSystem::Linux;
#else
    return QVariant();
#endif
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

QVariant ScriptApi::GetLineInfo(int lineNumber, int infoType) const {
  const QTextBlock block = cursor.document()->findBlockByLineNumber(lineNumber);
  if (!block.isValid())
    return QVariant();
  const int lineIndex = lineNumber - block.firstLineNumber();
  switch (infoType) {
  case 1: {
    const QTextLine line = block.layout()->lineAt(lineIndex);
    return block.text().sliced(line.textStart(), line.textLength());
  }
  case 2:
    return block.layout()->lineAt(lineIndex).textLength();
  case 3:
    return lineIndex == block.lineCount() - 1;
  case 4:
    return getLineType(block.charFormat()) == LineType::Note;
  case 5:
    return getLineType(block.charFormat()) == LineType::Input;
  case 6: // true if line logged
    return true;
  case 7: // true if bookmarked
    return false;
  case 8: {
    const QTextLine line = block.layout()->lineAt(lineIndex);
    return block.text().sliced(line.textStart(), line.textLength()) ==
           QStringLiteral("<hr>");
  }
  case 9:
    return getTimestamp(block.blockFormat());
  case 10:
    return lineNumber;
  case 11: {
    const QTextLine line = block.layout()->lineAt(lineIndex);
    int styleCount = 0;
    const int start = line.textStart();
    const int end = start + line.textLength();
    for (const QTextLayout::FormatRange &range : block.textFormats()) {
      if (range.start > end)
        return styleCount;
      if (range.start + range.length >= start)
        styleCount += 1;
    }
  }
  // case 12: // ticks - exact value from the high-performance timer
  case 13:
    return whenConnected.secsTo(getTimestamp(block.blockFormat()));
  default:
    return QVariant();
  }
}

QVariant ScriptApi::GetPluginInfo(string_view pluginID, int infoType) const {
  const size_t index = findPluginIndex(pluginID);
  if (index == noSuchPlugin || infoType < 0 || infoType > UINT8_MAX)
      [[unlikely]]
    return QVariant();
  switch (infoType) {
  case 16:
    return QVariant(!plugins[index].disabled());
  case 22:
    return QVariant(plugins[index].installed());
  default:
    return client()->pluginInfo(index, infoType);
  }
}

QVariant ScriptApi::GetStyleInfo(int line, int style, int infoType) const {
  const QTextDocument *doc = cursor.document();
  const QTextBlock block = doc->findBlockByLineNumber(line - 1);
  if (!block.isValid())
    return QVariant();
  const QTextLayout *layout = block.layout();
  const QTextLine textLine = layout->lineAt(line - block.firstLineNumber());
  const int textStart = textLine.textStart();
  const int textEnd = textStart + textLine.textLength();
  const QList<QTextLayout::FormatRange> styles = block.textFormats();
  auto iter = styles.cbegin();
  int styleOffset = style;
  for (auto end = styles.cend();; ++iter) {
    if (iter == end || iter->start > textEnd)
      return QVariant();
    if (iter->start + iter->length < textStart)
      continue;
    if (styleOffset == 0)
      break;
    --styleOffset;
  }
  const QTextLayout::FormatRange &range = *iter;
  const int rangeStart = std::max(textStart, range.start);
  const int rangeEnd = std::min(textEnd, range.start + range.length);
  switch (infoType) {
  case 1:
    return block.text().sliced(rangeStart, rangeEnd);
  case 2:
    return rangeEnd - rangeStart;
  case 3:
    return range.start - textStart;
  case 4: {
    optional<SendTo> sendto = getSendTo(range.format);
    if (!sendto)
      return 0;
    switch (*sendto) {
    case SendTo::Internet:
      return 2;
    case SendTo::World:
      return 1;
    case SendTo::Input:
      return 3;
    }
  }
  case 5: {
    QString link = range.format.anchorHref();
    if (!link.isEmpty())
      decodeLink(link);
    return link;
  }
  case 6:
    return range.format.toolTip();
  // case 7: // variable to set
  case 8:
    return range.format.fontWeight() == QFont::Weight::Bold;
  case 9:
    return range.format.fontUnderline();
  case 10:
    return getStyles(range.format).testFlag(TextStyle::Blink);
  case 11:
    return getStyles(range.format).testFlag(TextStyle::Inverse);
  case 12: // changed by trigger from original
    return false;
  case 13: // true if start of a tag (action is tag name)
    return false;
  case 14:
    return range.format.foreground().color();
  case 15:
    return range.format.background().color();
  default:
    return QVariant();
  }
}

QVariant ScriptApi::GetTimerInfo(size_t pluginIndex, const QString &label,
                                 int infoType) const {
  if (infoType < 0 || infoType > UINT8_MAX) [[unlikely]]
    return QVariant();

  switch (infoType) {
  case 26: {
    const QString scriptName =
        client()->timerInfo(pluginIndex, label, 5).toString();
    return !scriptName.isEmpty() &&
           plugins[pluginIndex].hasFunction(scriptName);
  }
  default:
    return client()->timerInfo(pluginIndex, label, infoType);
  }
}

// External implementations

QVariant MiniWindow::info(int infoType) const {
  switch (infoType) {
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
    return geometry().left();
  case 11:
    return geometry().top();
  case 12:
    return geometry().right();
  case 13:
    return geometry().bottom();
  case 14:
    return mapFromGlobal(QCursor::pos()).x();
  case 15:
    return mapFromGlobal(QCursor::pos()).y();
  case 16:
    return 1; // incremented each time items 14/15 are updated
  case 17:
    return parentWidget()->mapFromGlobal(QCursor::pos()).x();
  case 18:
    return parentWidget()->mapFromGlobal(QCursor::pos()).y();
  case 19:
    return QString(); // hotspot currently being moused-over in
  case 20:
    return QString(); // hotspot currently being moused-down in
  case 21:
    return installed;
  case 22:
    return zOrder;
  case 23:
    return QString::fromUtf8(pluginID.data(), pluginID.size());
  default:
    return QVariant();
  }
}

QVariant Hotspot::info(int infoType) const {
  switch (infoType) {
  case 1:
    return geometry().left();
  case 2:
    return geometry().top();
  case 3:
    return geometry().right();
  case 4:
    return geometry().bottom();
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
    switch (cursor().shape()) {
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
