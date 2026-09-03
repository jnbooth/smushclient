#include "settings.h"
#include "casting.h"
#include "environment.h"
#include <QtGui/QColor>
#include <QtGui/QFontDatabase>
#include <type_traits>

using Qt::StringLiterals::operator""_L1;

#if defined(Q_OS_WIN)
#define DIR_SEP "\\"
#else
#define DIR_SEP "/"
#endif

template<typename T>
using pass_by_t =
  std::conditional_t<std::is_trivially_copy_constructible_v<T> &&
                       sizeof(T) <= 8,
                     T,
                     const T&>;

#define SETTING(name, T, defaultValue, key)                                    \
  void Settings::set##name(pass_by_t<T> value)                                 \
  {                                                                            \
    store.setValue(key ""_L1, value);                                          \
  }                                                                            \
                                                                               \
  T Settings::get##name() const                                                \
  {                                                                            \
    const QVariant value = store.value(key ""_L1);                             \
    if (value.isNull()) {                                                      \
      return defaultValue;                                                     \
    }                                                                          \
    return qvariant_cast<T>(value);                                            \
  }

#define SETTING_ENUM(name, T, defaultValue, key)                               \
  void Settings::set##name(T value)                                            \
  {                                                                            \
    store.setValue(key ""_L1, to_underlying(value));                           \
  }                                                                            \
                                                                               \
  T Settings::get##name() const                                                \
  {                                                                            \
    const QVariant value = store.value(key ""_L1);                             \
    if (value.isNull()) {                                                      \
      return defaultValue;                                                     \
    }                                                                          \
    return qvariant_enum_cast<T>(value);                                       \
  }

// Private utils

namespace {
QFont
getDefaultFont(int pointSize)
{
  QFont defaultFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
  defaultFont.setPointSize(pointSize);
  return defaultFont;
}
} // namespace

// Public methods

Settings::Settings(QObject* parent)
  : QObject(parent)
{
}

QString
Settings::fileName() const
{
  return store.fileName();
}

// Dynamic

QTextBlockFormat
Settings::getOutputBlockFormat() const
{
  const int spacing = getOutputLineSpacing();
  const QTextBlockFormat::LineHeightTypes spacingType =
    spacing == 100 ? QTextBlockFormat::LineHeightTypes::SingleHeight
                   : QTextBlockFormat::LineHeightTypes::ProportionalHeight;
  QTextBlockFormat format;
  format.setLineHeight(spacing, spacingType);
  return format;
}

QPalette
Settings::getInputPalette() const
{
  QPalette palette;
  palette.setColor(QPalette::ColorRole::Text, getInputForeground());
  palette.setColor(QPalette::ColorRole::Base, getInputBackground());
  return palette;
}

QPalette
Settings::getNotepadPalette() const
{
  QPalette palette;
  palette.setColor(QPalette::ColorRole::Text, getNotepadForeground());
  palette.setColor(QPalette::ColorRole::Base, getNotepadBackground());
  return palette;
}

// Recent files

constexpr QLatin1StringView recentFilesKey = "recent"_L1;

constexpr qsizetype recentFilesMax = 5;

QStringList
Settings::getRecentFiles() const
{
  QStringList recent = store.value(recentFilesKey).toStringList();
  // Should never happen, but just in case.
  if (recent.size() > recentFilesMax) {
    recent.resize(recentFilesMax);
  }
  return recent;
}

RecentFileResult
Settings::addRecentFile(const QString& path)
{
  QString relativePath = makePathRelative(path);
  QStringList recent = getRecentFiles();
  qsizetype index = recent.indexOf(relativePath);
  if (index == 0) {
    return RecentFileResult{ .changed = false, .recentFiles = recent };
  }

  if (index != -1) {
    recent.remove(index);
  } else if (recent.size() == recentFilesMax) {
    recent.removeLast();
  }

  recent.prepend(relativePath);
  store.setValue(recentFilesKey, recent);
  return RecentFileResult{ .changed = true, .recentFiles = recent };
}

RecentFileResult
Settings::removeRecentFile(const QString& path)
{
  QString relativePath = makePathRelative(path);
  QStringList recent = getRecentFiles();
  qsizetype index = recent.indexOf(relativePath);
  if (index == -1) {
    return RecentFileResult{ .changed = false, .recentFiles = recent };
  }

  recent.remove(index);
  store.setValue(recentFilesKey, recent);
  return RecentFileResult{ .changed = true, .recentFiles = recent };
}

QStringList
Settings::getStartupWorlds() const
{
  switch (getStartupBehavior()) {
    case StartupBehavior::Reopen:
      return getLastFiles();
    case StartupBehavior::None:
      return QStringList();
    case StartupBehavior::List:
      return getOpenAtStartup();
  }
}

QString
Settings::getStartupDirectoryOrDefault() const
{
  const QString dir = getStartupDirectory();
  return dir.isEmpty() ? defaultStartupDirectory() : dir;
}

QString
Settings::getLogsDir() const
{
  return getStartupDirectoryOrDefault() + DIR_SEP LOGS_DIR ""_L1;
}

QString
Settings::getPluginsDir() const
{
  return getStartupDirectoryOrDefault() + DIR_SEP PLUGINS_DIR ""_L1;
}

QString
Settings::getScriptsDir() const
{
  return getStartupDirectoryOrDefault() + DIR_SEP SCRIPTS_DIR ""_L1;
}

QString
Settings::getSoundsDir() const
{
  return getStartupDirectoryOrDefault() + DIR_SEP SOUNDS_DIR ""_L1;
}

QString
Settings::getWorldsDir() const
{
  return getStartupDirectoryOrDefault() + DIR_SEP WORLDS_DIR ""_L1;
}

// Generated

SETTING(AutoConnect, bool, true, "connecting/auto");

SETTING(BackgroundMaterial, int, 12, "background/material");
SETTING(BackgroundTransparent, bool, false, "background/transparent");

SETTING(BellSound, QString, {}, "sound/bell");

SETTING(ConfirmQuit, bool, false, "closing/app");

SETTING(DisplayConnect, bool, true, "connecting/display/connect");
SETTING(DisplayDisconnect, bool, true, "connecting/display/disconnect");

SETTING(InputBackground, QColor, Qt::white, "input/background");
SETTING(InputFont, QFont, getDefaultFont(12), "input/font");
SETTING(InputForeground, QColor, Qt::black, "input/foreground");
SETTING(InputHistoryLimit, bool, true, "input/history/limit");
SETTING(InputHistoryLines, int, 100, "input/history/lines");

SETTING(LastFiles, QStringList, {}, "startup/reopen");

SETTING(LoggingEnabled, bool, true, "logging/enable");

SETTING(NotepadFont, QFont, getDefaultFont(12), "notepad/font");
SETTING(NotepadBackground, QColor, Qt::white, "notepad/background");
SETTING(NotepadForeground, QColor, Qt::black, "notepad/foreground");

SETTING(OpenAtStartup, QStringList, {}, "startup/list");

SETTING(OutputFont, QFont, getDefaultFont(12), "output/font");
SETTING(OutputHistoryEnabled, bool, true, "output/history/enable");
SETTING(OutputHistoryLimit, bool, true, "output/history/limit");
SETTING(OutputHistoryLines, int, 1000, "output/history/lines");
SETTING(OutputPadding, double, 0.0, "output/inset");
SETTING(OutputLimit, bool, false, "output/limit");
SETTING(OutputLines, int, 10000, "output/lines");
SETTING(OutputLineSpacing, int, 100.0, "output/spacing");
SETTING(OutputWrapping, bool, true, "output/wrap");

SETTING(ReconnectOnDisconnect, bool, false, "connecting/reconnect");

SETTING(ScriptFont, QFont, getDefaultFont(12), "script/font");

SETTING(ShowInfoBar, bool, false, "infobar/visible");

SETTING(ShowStatusBar, bool, true, "statusbar/visible");

SETTING_ENUM(StartupBehavior,
             Settings::StartupBehavior,
             StartupBehavior::Reopen,
             "startup/behavior");
SETTING(StartupDirectory, QString, {}, "startup/directory");

SETTING(TimestampFormat,
        QString,
        QStringLiteral("hh:mm:ss MM/dd"),
        "output/timestamp/format");

SETTING_ENUM(WorldCloseBehavior,
             Settings::WorldCloseBehavior,
             WorldCloseBehavior::Confirm,
             "closing/world");
