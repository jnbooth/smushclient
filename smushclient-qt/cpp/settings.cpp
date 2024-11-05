#include "settings.h"
#include <QtCore/QDir>
#include <QtCore/QUuid>
#include <QtGui/QColor>
#include <QtGui/QFontDatabase>

QFont getDefaultFont(int pointSize)
{
  QFont defaultFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
  defaultFont.setPointSize(pointSize);
  return defaultFont;
}

// Private utilities

QString makePathRelative(const QString &filePath)
{
  const QString cwd = QDir::currentPath();
  if (!filePath.startsWith(cwd))
    return filePath;
  const qsizetype cwdSize = cwd.size() + 1;
  return filePath.sliced(cwdSize, filePath.size() - cwdSize);
}

// Public methods

Settings::Settings() : store() {}

// Auto connect

static const QString autoConnectKey = QStringLiteral("connecting/auto");

bool Settings::autoConnect() const
{
  return value(autoConnectKey, true);
}

void Settings::setAutoConnect(bool enabled)
{
  store.setValue(autoConnectKey, enabled);
}

// Input font

static const QString inputFontKey = QStringLiteral("input/font");

QFont Settings::inputFont() const
{
  static const QFont defaultFont = getDefaultFont(12);
  return value(inputFontKey, defaultFont);
}

void Settings::setInputFont(const QFont &font)
{
  store.setValue(inputFontKey, font);
}

// Input background

static const QString inputBackgroundKey = QStringLiteral("input/background");

QColor Settings::inputBackground() const
{
  return value(inputBackgroundKey, QColor(Qt::GlobalColor::white));
}

void Settings::setInputBackground(const QColor &color)
{
  store.setValue(inputBackgroundKey, color);
}

// Input foreground

static const QString inputForegroundKey = QStringLiteral("input/foreground");

QColor Settings::inputForeground() const
{
  return value(inputForegroundKey, QColor(Qt::GlobalColor::black));
}

void Settings::setInputForeground(const QColor &color)
{
  store.setValue(inputForegroundKey, color);
}

// Last files

static const QString lastFilesKey = QStringLiteral("reopen");

QStringList Settings::lastFiles() const
{
  return store.value(lastFilesKey).toStringList();
}

void Settings::setLastFiles(const QStringList &files)
{
  return store.setValue(lastFilesKey, files);
}

// Logging enabled

static const QString loggingEnabledKey = QStringLiteral("logging/enable");

bool Settings::loggingEnabled() const
{
  return value(loggingEnabledKey, true);
}

void Settings::setLoggingEnabled(bool enabled)
{
  store.setValue(loggingEnabledKey, enabled);
}

// Output font

static const QString outputFontKey = QStringLiteral("output/font");

QFont Settings::outputFont() const
{
  static const QFont defaultFont = getDefaultFont(12);
  return value(outputFontKey, defaultFont);
}

void Settings::setOutputFont(const QFont &font)
{
  store.setValue(outputFontKey, font);
}

// Output wrapping

static const QString outputWrapKey = QStringLiteral("output/wrap");

bool Settings::outputWrapping() const
{
  return value(outputWrapKey, true);
}

void Settings::setOutputWrapping(bool wrapping)
{
  store.setValue(outputWrapKey, wrapping);
}

// Recent files

static const QString recentFilesKey = QStringLiteral("recent");

constexpr qsizetype recentFilesMax = 5;

QStringList Settings::recentFiles() const
{
  QStringList recent = store.value(recentFilesKey).toStringList();
  // Should never happen, but just in case.
  if (recent.size() > recentFilesMax)
    recent.resize(recentFilesMax);
  return recent;
}

RecentFileResult Settings::addRecentFile(const QString &path)
{
  QString relativePath = makePathRelative(path);
  QStringList recent = recentFiles();
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

RecentFileResult Settings::removeRecentFile(const QString &path)
{
  QString relativePath = makePathRelative(path);
  QStringList recent = recentFiles();
  qsizetype index = recent.indexOf(relativePath);
  if (index == -1)
    return RecentFileResult{.changed = false, .recentFiles = recent};

  recent.remove(index);
  store.setValue(recentFilesKey, recent);
  return RecentFileResult{.changed = true, .recentFiles = recent};
}

// Reconnect on disconnect

static const QString reconnectKey = QStringLiteral("connecting/reconnect");

bool Settings::reconnectOnDisconnect() const
{
  return value(reconnectKey, false);
}

void Settings::setReconnectOnDisconnect(bool enabled)
{
  store.setValue(reconnectKey, enabled);
}

// Show status bar

static const QString showStatusBarKey = QStringLiteral("statusbar/visible");

bool Settings::showStatusBar() const
{
  return value(showStatusBarKey, true);
}

void Settings::setShowStatusBar(bool show)
{
  store.setValue(showStatusBarKey, show);
}
