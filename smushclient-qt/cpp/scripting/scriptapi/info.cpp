#include "../../environment.h"
#include "../../settings.h"
#include "../../spans.h"
#include "../../ui/components/mudscrollbar.h"
#include "../../ui/mudstatusbar/mudstatusbar.h"
#include "../../ui/ui_worldtab.h"
#include "../../ui/worldtab.h"
#include "../scriptapi.h"
#include "smushclient_qt/src/ffi/spans.cxx.h"
#include "smushclient_qt/src/ffi/util.cxx.h"
#include <QtCore/QDir>
#include <QtCore/QOperatingSystemVersion>
#include <QtCore/QTranslator>
#include <QtGui/QTextBlock>
#include <QtGui/QTextDocumentFragment>
#include <QtGui/QTextLayout>
#include <QtGui/QTextLine>
#include <QtNetwork/QHostInfo>
#include <QtNetwork/QNetworkProxy>
#include <QtWidgets/QTabWidget>

using std::string;
using std::string_view;

// Private utils

enum ModifierFlag : int64_t
{
  Shift = 0x00001,
  Ctrl = 0x00002,
  Alt = 0x00004,
  LeftShift = 0x00008,
  RightShift = 0x00010,
  LeftCtrl = 0x00020,
  RightCtrl = 0x00040,
  LeftAlt = 0x00080,
  RightAlt = 0x00100,
  CapsLock = 0x00200,
  NumLock = 0x00400,
  ScrollLock = 0x00800,
  CapsLockToggled = 0x02000,
  NumLockToggled = 0x04000,
  ScrollLockToggled = 0x08000,
  LeftMouse = 0x10000,
  RightMouse = 0x20000,
  MiddleMouse = 0x40000,
};
Q_DECLARE_FLAGS(ModifierFlags, ModifierFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(ModifierFlags)

namespace {
QString
defaultPath(const QString& name)
{
  return QDir::currentPath() + QDir::separator() + name;
}

constexpr ModifierFlags
getModifierFlags(Qt::KeyboardModifiers mods, Qt::MouseButtons buttons)
{
  ModifierFlags flags;

  if (mods.testFlag(Qt::KeyboardModifier::ShiftModifier)) {
    flags |= ModifierFlag::Shift | ModifierFlag::LeftShift;
  }
  if (mods.testFlag(Qt::KeyboardModifier::ControlModifier)) {
    flags |= ModifierFlag::Ctrl | ModifierFlag::LeftCtrl;
  }
  if (mods.testFlag(Qt::KeyboardModifier::AltModifier)) {
    flags |= ModifierFlag::Alt | ModifierFlag::LeftAlt;
  }

  if (buttons.testFlag(Qt::MouseButton::LeftButton)) {
    flags |= ModifierFlag::LeftMouse;
  }
  if (buttons.testFlag(Qt::MouseButton::RightButton)) {
    flags |= ModifierFlag::RightMouse;
  }
  if (buttons.testFlag(Qt::MouseButton::MiddleButton)) {
    flags |= ModifierFlag::MiddleMouse;
  }

  return flags;
}

constexpr ScriptBrush
getBrushStyle(const QBrush& brush)
{
  switch (brush.style()) {
    case Qt::BrushStyle::SolidPattern:
      return ScriptBrush::SolidPattern;
    case Qt::BrushStyle::NoBrush:
      return ScriptBrush::NoBrush;
    case Qt::BrushStyle::HorPattern:
      return ScriptBrush::HorPattern;
    case Qt::BrushStyle::VerPattern:
      return ScriptBrush::VerPattern;
    case Qt::BrushStyle::FDiagPattern:
      return ScriptBrush::FDiagPattern;
    case Qt::BrushStyle::BDiagPattern:
      return ScriptBrush::BDiagPattern;
    case Qt::BrushStyle::CrossPattern:
      return ScriptBrush::CrossPattern;
    case Qt::BrushStyle::DiagCrossPattern:
      return ScriptBrush::DiagCrossPattern;
    case Qt::BrushStyle::Dense4Pattern:
      return ScriptBrush::Dense4Pattern;
    case Qt::BrushStyle::Dense2Pattern:
      return ScriptBrush::Dense2Pattern;
    case Qt::BrushStyle::Dense1Pattern:
      return ScriptBrush::Dense1Pattern;
    default:
      return ScriptBrush::SolidPattern;
  }
}
} // namespace

// Public static methods

bool
ScriptApi::ChangeDir(const QString& dir)
{
  return QDir::setCurrent(dir);
}

QVariant
ScriptApi::FontInfo(const QFont& font, int64_t infoType)
{
  if (infoType != 19) {
    return ffi::util::font_info(font, infoType);
  }
  const ScriptFont scriptFont(font);
  return static_cast<int64_t>(ScriptFont(font));
}

int64_t
ScriptApi::GetUniqueNumber() noexcept
{
  static int64_t uniqueNumber = -1;
  if (uniqueNumber == INT64_MAX) [[unlikely]] {
    uniqueNumber = 0;
  } else {
    ++uniqueNumber;
  }
  return uniqueNumber;
}

// Public methods

QElapsedTimer::Duration
ScriptApi::GetConnectDuration() const
{
  if (!whenConnected.isValid()) {
    return {};
  }
  return whenConnected.durationElapsed();
}

QVariant
ScriptApi::GetInfo(int64_t infoType) const
{
  switch (infoType) {
    case 20:
      return tab.ui->output->fontFamily();
    case 23:
      return tab.ui->input->fontFamily();
    case 50:
      return Settings().getBellSound();
    case 53:
      return statusBar()->message();
    case 54:
      return tab.worldFilePath();
    case 55:
      return tab.title();
    case 56:
      return QCoreApplication::applicationFilePath();
    case 57:
      return defaultPath(QStringLiteral(WORLDS_DIR));
    case 58:
      return defaultPath(QStringLiteral(LOGS_DIR));
    case 59:
      return defaultPath(QStringLiteral(SCRIPTS_DIR));
    case 60:
      return defaultPath(QStringLiteral(PLUGINS_DIR));
    case 61:
      return socket.peerAddress().toString();
    case 62:
      return socket.proxy().hostName();
    case 63:
      return QHostInfo::localHostName();
    case 64:
      return QDir::currentPath();
    case 66:
      return QCoreApplication::applicationDirPath();
    case 67:
      return QFileInfo(tab.worldFilePath()).absoluteDir().path();
    case 68:
      return QDir::currentPath(); // initial directory
    case 69:
      return QTranslator().filePath();
    case 70:
      return QLocale().name();
    case 71:
      return QFontDatabase::systemFont(QFontDatabase::SystemFont::FixedFont)
        .family();
    case 72:
      return QStringLiteral(SCRIPTING_VERSION);
    case 73:
      return QStringLiteral(__DATE__ " " __TIME__);
    case 74:
      return defaultPath(QStringLiteral(SOUNDS_DIR));
    // case 76: Special font pathname
    // case 77: Windows version debug string
    case 77:
      return QOperatingSystemVersion::current().name();
    // case 78: Foreground image name
    // case 79: Background image name
    // case 80: libpng version
    // case 81: libpng header
    case 82:
      return Settings().fileName();
    case 83:
      return QString::fromUtf8(sqlite3_libversion());
    case 84:
      return QDir::currentPath(); // File-browsing directory
    case 86:
      return wordUnderMenu;
    case 87:
      return lastCommandSent;
    case 101:
      return cursor->suppressingEcho();
    case 106:
      return !socket.isOpen();
    case 107:
      return socket.state() == QAbstractSocket::SocketState::ConnectingState;
    case 108: // OK to disconnect
      return true;
    case 111:
      return tab.isWindowModified();
    // case 112: Automapper active
    case 113:
      return tab.active();
    case 114:
      return scrollBar.paused();
    // case 115: Localization active
    case 119: // Script engine is active
      return true;
    case 120:
      return scrollBar.isVisible();
    case 121: // High-resolution timer is available
      return true;
    case 122:
      return sqlite3_threadsafe() != 0;
    case 125:
      return tab.isFullScreen();
    case 203:
      return totalLinesSent;
    case 204:
      return totalPacketsSent;
    case 212:
      return QFontMetrics(tab.ui->output->font()).height();
    case 213:
      return QFontMetrics(tab.ui->output->font()).maxWidth();
    case 214:
      return QFontMetrics(tab.ui->input->font()).height();
    case 215:
      return QFontMetrics(tab.ui->input->font()).maxWidth();
    case 217:
      return totalBytesSent;
    case 224: {
      MudBrowser& output = *tab.ui->output;
      const QRect rect = output.rect();
      const QTextCursor firstVisible = output.cursorForPosition(rect.topLeft());
      const QTextCursor lastVisible =
        output.cursorForPosition(rect.bottomRight());
      return lastVisible.blockNumber() - firstVisible.blockNumber();
    }
    case 227:
      switch (socket.state()) {
        case QAbstractSocket::SocketState::HostLookupState:
          return 1;
        case QAbstractSocket::SocketState::ConnectingState:
          return 3;
        case QAbstractSocket::SocketState::ConnectedState:
          return 8;
        default:
          return 0;
      }
    case 228: {
      bool isOk;
      const uint32_t ipv4 = socket.peerAddress().toIPv4Address(&isOk);
      return isOk ? ipv4 : QVariant();
    }
    // case 229: Proxy IP address as number
    // case 230: Script execution depth
    case 235:
      return qobject_cast<QTabWidget*>(tab.parent())->count();
    case 236: {
      const QTextCursor inputCursor = tab.ui->input->textCursor();
      return std::min(inputCursor.anchor(), inputCursor.position());
    }
    case 237: {
      const QTextCursor inputCursor = tab.ui->input->textCursor();
      return std::max(inputCursor.anchor(), inputCursor.position());
    }
    case 238: {
      QWidget& window = *tab.window();
      if (window.isHidden()) {
        return 0;
      }
      if (window.isMinimized()) {
        return 6;
      }
      if (window.isMaximized()) {
        return 3;
      }
      return 1;
    }
    case 239:
      return static_cast<int>(actionSource);
    case 240:
      return QFontMetrics(tab.ui->output->font()).averageCharWidth();
    case 241:
      return QFontMetrics(tab.ui->output->font()).height();
    case 249:
      return tab.window()->height();
    case 250:
      return tab.window()->width();
    case 259:
      return statusBar()->height();
    case 260:
      return statusBar()->width();
    case 261:
      return tab.ui->outputBorder->height();
    case 262:
      return tab.ui->outputBorder->width();
    case 263:
      return tab.ui->outputBorder->height();
    case 264:
      return tab.ui->outputBorder->width();
    case 268:
#if defined(Q_OS_WIN)
      return static_cast<int>(OperatingSystem::Windows);
#elif defined(Q_OS_MACOS)
      return static_cast<int>(OperatingSystem::MacOS);
#elif defined(Q_OS_LINUX)
      return static_cast<int>(OperatingSystem::Linux);
#else
      return QVariant();
#endif
    case 271:
      return tab.ui->output->palette().brush(QPalette::ColorRole::Base).color();
    case 272:
      return assignedTextRectangle.left();
    case 273:
      return assignedTextRectangle.top();
    case 274:
      return assignedTextRectangle.right();
    case 275:
      return assignedTextRectangle.bottom();
    case 276:
      return tab.ui->background->contentsMargins().left();
    case 277:
      return tab.ui->outputBorder->contentsMargins().left();
    case 278:
      return tab.ui->area->palette().brush(QPalette::ColorRole::Window).color();
    case 279:
      return static_cast<int>(getBrushStyle(
        tab.ui->area->palette().brush(QPalette::ColorRole::Window)));
    case 280:
      return tab.ui->area->height();
    case 281:
      return tab.ui->area->width();
    case 282:
      return tab.ui->outputBorder->palette()
        .brush(QPalette::ColorRole::Window)
        .color();
    case 283:
      return tab.ui->area->mapFromGlobal(QCursor::pos()).x();
    case 284:
      return tab.ui->area->mapFromGlobal(QCursor::pos()).y();
    case 285: // Is output window
      return true;
    case 290:
      return tab.ui->area->contentsMargins().left();
    case 291:
      return tab.ui->area->contentsMargins().top();
    case 292: {
      QWidget& area = *tab.ui->area;
      return area.width() - area.contentsMargins().right();
    }
    case 293: {
      QWidget& area = *tab.ui->area;
      return area.height() - area.contentsMargins().bottom();
    }
    case 294:
      return getModifierFlags(QGuiApplication::keyboardModifiers(),
                              QGuiApplication::mouseButtons())
        .toInt();
    case 296:
      return scrollBar.sliderPosition();
    case 298:
      return sqlite3_libversion_number();
    default:
      return client.getInfo(infoType);
  }
}

QVariant
ScriptApi::GetLineInfo(int lineNumber, int64_t infoType) const
{
  const QTextBlock block =
    cursor->document()->findBlockByLineNumber(lineNumber);
  if (!block.isValid()) {
    return QVariant();
  }
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
      return spans::getLineType(block.charFormat()) == LineType::Note;
    case 5:
      return spans::getLineType(block.charFormat()) == LineType::Input;
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
      return spans::getTimestamp(block.blockFormat());
    case 10:
      return lineNumber;
    case 11: {
      const QTextLine line = block.layout()->lineAt(lineIndex);
      int styleCount = 0;
      const int start = line.textStart();
      const int end = start + line.textLength();
      for (const QTextLayout::FormatRange& range : block.textFormats()) {
        if (range.start > end) {
          return styleCount;
        }
        if (range.start + range.length >= start) {
          ++styleCount;
        }
      }
    }
    // case 12: // ticks - exact value from the high-performance timer
    case 13:
      return spans::getElapsed(block.blockFormat()).msecsSinceReference();
    default:
      return QVariant();
  }
}

QVariant
ScriptApi::GetPluginInfo(string_view pluginID, int64_t infoType) const noexcept
{
  const size_t index = findPluginIndex(pluginID);
  if (index == noSuchPlugin) [[unlikely]] {
    return {};
  }
  switch (infoType) {
    case 16:
      return QVariant(!plugins[index].isDisabled());
    case 22:
      return QVariant(plugins[index].installed());
    default:
      return client.pluginInfo(index, infoType);
  }
}

QVariant
ScriptApi::GetStyleInfo(int line, int64_t style, int64_t infoType) const
{
  const QTextDocument& doc = *cursor->document();
  const QTextBlock block = doc.findBlockByLineNumber(line - 1);
  if (!block.isValid()) {
    return QVariant();
  }
  const QTextLayout* layout = block.layout();
  if (layout == nullptr) [[unlikely]] {
    return QVariant();
  }
  const QTextLine textLine = layout->lineAt(line - block.firstLineNumber());
  const int textStart = textLine.textStart();
  const int textEnd = textStart + textLine.textLength();
  const QList<QTextLayout::FormatRange> styles = block.textFormats();
  auto iter = styles.cbegin();
  int64_t styleOffset = style;
  for (auto end = styles.cend();; ++iter) {
    if (iter == end || iter->start > textEnd) {
      return QVariant();
    }
    if (iter->start + iter->length < textStart) {
      continue;
    }
    if (styleOffset == 0) {
      break;
    }
    --styleOffset;
  }
  const QTextLayout::FormatRange& range = *iter;
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
      if (range.format.anchorHref().isEmpty()) {
        return 0;
      }
      SendTo sendto = spans::getSendTo(range.format);
      switch (sendto) {
        case SendTo::Internet:
          return 2;
        case SendTo::World:
          return 1;
        case SendTo::Input:
          return 3;
      }
    }
    case 5:
      return range.format.anchorHref();
    case 6:
      return range.format.toolTip();
    // case 7: // variable to set
    case 8:
      return range.format.fontWeight() == QFont::Weight::Bold;
    case 9:
      return range.format.fontUnderline();
    case 10:
      return spans::getStyles(range.format).testFlag(TextStyle::Blink);
    case 11:
      return spans::getStyles(range.format).testFlag(TextStyle::Inverse);
    case 12: // changed by trigger from original
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

// External implementations

QVariant
MiniWindow::info(int64_t infoType) const
{
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
      return static_cast<int>(position);
    case 8:
      return static_cast<int>(flags);
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
    case 19: // hotspot currently being moused-over in
    case 20: // hotspot currently being moused-down in
      return QString();
    case 21:
      return installed;
    case 22:
      return zOrder;
    case 23:
      return QString::fromUtf8(pluginID);
    default:
      return QVariant();
  }
}

QVariant
Hotspot::info(int64_t infoType) const
{
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
      return QString::fromUtf8(callbacks.mouseOver);
    case 6:
      return QString::fromUtf8(callbacks.cancelMouseOver);
    case 7:
      return QString::fromUtf8(callbacks.mouseDown);
    case 8:
      return QString::fromUtf8(callbacks.cancelMouseDown);
    case 9:
      return QString::fromUtf8(callbacks.mouseUp);
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
      return (underMouse() ? 0x80 : 0) + (hasMouseTracking() ? 1 : 0);
    case 13:
      return QString::fromUtf8(callbacks.dragMove);
    case 14:
      return QString::fromUtf8(callbacks.dragRelease);
    case 15: // DragHandler flags
      return 0;
    default:
      return QVariant();
  }
}
