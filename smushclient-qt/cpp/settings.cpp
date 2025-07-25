#include "settings.h"
#include "environment.h"
#include <QtGui/QColor>
#include <QtGui/QFontDatabase>

// clang-format off
template <typename T> struct SettingInput    { typedef T Type; };
template <> struct SettingInput<QColor>      { typedef const QColor &Type; };
template <> struct SettingInput<QFont>       { typedef const QFont &Type; };
template <> struct SettingInput<QString>     { typedef const QString &Type; };
template <> struct SettingInput<QStringList> { typedef const QStringList &Type; };
// clang-format on

#define SETTING(NAME, T, DEFAULT, KEY)                                         \
  static const QString key##NAME = QStringLiteral(KEY);                        \
  void Settings::set##NAME(SettingInput<T>::Type value) {                      \
    store.setValue(key##NAME, value);                                          \
  }                                                                            \
  T Settings::get##NAME() const {                                              \
    return store.contains(key##NAME) ? store.value(key##NAME).value<T>()       \
                                     : DEFAULT;                                \
  }

#define SETTING_ENUM(NAME, T, DEFAULT, KEY)                                    \
  static const QString key##NAME = QStringLiteral(KEY);                        \
  void Settings::set##NAME(SettingInput<T>::Type value) {                      \
    store.setValue(key##NAME, (int)value);                                     \
  }                                                                            \
  T Settings::get##NAME() const {                                              \
    return store.contains(key##NAME) ? (T)store.value(key##NAME).value<T>()    \
                                     : DEFAULT;                                \
  }

// Private utils

QFont getDefaultFont(int pointSize) {
  QFont defaultFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
  defaultFont.setPointSize(pointSize);
  return defaultFont;
}

static const QFont defaultFont = getDefaultFont(12);

const QString headerKey(const QString &modelName) {
  return QStringLiteral("headers/") + modelName;
}

// Public methods

Settings::Settings(QObject *parent) : QObject(parent) {}

// Dynamic

QTextBlockFormat Settings::getOutputBlockFormat() const {
  const int spacing = getOutputLineSpacing();
  const QTextBlockFormat::LineHeightTypes spacingType =
      spacing == 100 ? QTextBlockFormat::LineHeightTypes::SingleHeight
                     : QTextBlockFormat::LineHeightTypes::ProportionalHeight;
  QTextBlockFormat format;
  format.setLineHeight(spacing, spacingType);
  return format;
}

// Recent files

static const QString recentFilesKey = QStringLiteral("recent");

constexpr qsizetype recentFilesMax = 5;

QStringList Settings::getRecentFiles() const {
  QStringList recent = store.value(recentFilesKey).toStringList();
  // Should never happen, but just in case.
  if (recent.size() > recentFilesMax)
    recent.resize(recentFilesMax);
  return recent;
}

RecentFileResult Settings::addRecentFile(const QString &path) {
  QString relativePath = makePathRelative(path);
  QStringList recent = getRecentFiles();
  qsizetype index = recent.indexOf(relativePath);
  if (index == 0)
    return RecentFileResult{.changed = false, .recentFiles = recent};

  if (index != -1)
    recent.remove(index);
  else if (recent.size() == recentFilesMax)
    recent.removeLast();

  recent.prepend(relativePath);
  store.setValue(recentFilesKey, recent);
  return RecentFileResult{.changed = true, .recentFiles = recent};
}

RecentFileResult Settings::removeRecentFile(const QString &path) {
  QString relativePath = makePathRelative(path);
  QStringList recent = getRecentFiles();
  qsizetype index = recent.indexOf(relativePath);
  if (index == -1)
    return RecentFileResult{.changed = false, .recentFiles = recent};

  recent.remove(index);
  store.setValue(recentFilesKey, recent);
  return RecentFileResult{.changed = true, .recentFiles = recent};
}

QStringList Settings::getStartupWorlds() const {
  switch (getStartupBehavior()) {
  case StartupBehavior::Reopen:
    return getLastFiles();
  case StartupBehavior::None:
    return QStringList();
  case StartupBehavior::List:
    return getOpenAtStartup();
  }
}

QString Settings::getStartupDirectoryOrDefault() const {
  const QString dir = getStartupDirectory();
  return dir.isEmpty() ? defaultStartupDirectory() : dir;
}

// Generated

SETTING(AutoConnect, bool, true, "connecting/auto");

SETTING(BellSound, QString, QString(), "sound/bell");

SETTING(ConfirmQuit, bool, false, "closing/app");

SETTING(DisplayConnect, bool, true, "connecting/display/connect");
SETTING(DisplayDisconnect, bool, true, "connecting/display/disconnect");

SETTING(InputBackground, QColor, QColor(Qt::GlobalColor::white),
        "input/background");
SETTING(InputFont, QFont, getDefaultFont(12), "input/font");
SETTING(InputForeground, QColor, QColor(Qt::GlobalColor::black),
        "input/foreground");
SETTING(InputHistoryLimit, bool, true, "input/history/limit");
SETTING(InputHistoryLines, int, 100, "input/history/lines");

SETTING(LastFiles, QStringList, QStringList(), "startup/reopen");

SETTING(LoggingEnabled, bool, true, "logging/enable");

SETTING(NotepadFont, QFont, getDefaultFont(12), "notepad/font");
SETTING(NotepadBackground, QColor, QColor(Qt::GlobalColor::white),
        "notepad/background");
SETTING(NotepadForeground, QColor, QColor(Qt::GlobalColor::black),
        "notepad/foreground");

SETTING(OpenAtStartup, QStringList, QStringList(), "startup/list");

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

SETTING(ShowStatusBar, bool, true, "statusbar/visible");

SETTING_ENUM(StartupBehavior, Settings::StartupBehavior,
             StartupBehavior::Reopen, "startup/behavior");
SETTING(StartupDirectory, QString, QString(), "startup/directory");

SETTING_ENUM(WorldCloseBehavior, Settings::WorldCloseBehavior,
             WorldCloseBehavior::Confirm, "closing/world");
