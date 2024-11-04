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

// Input font

static const QString inputFontKey = QStringLiteral("input/font");

QFont Settings::inputFont() const
{
  const QVariant font = store.value(inputFontKey);
  if (font.canConvert<QFont>())
    return font.value<QFont>();

  static const QFont defaultFont = getDefaultFont(12);
  return defaultFont;
}

void Settings::setInputFont(const QFont &font)
{
  store.setValue(inputFontKey, font);
}

// Input background

static const QString inputBackgroundKey = QStringLiteral("input/background");

QColor Settings::inputBackground() const
{
  const QVariant color = store.value(inputBackgroundKey);
  if (color.canConvert<QColor>())
    return color.value<QColor>();
  return Qt::GlobalColor::white;
}

void Settings::setInputBackground(const QColor &color)
{
  store.setValue(inputBackgroundKey, color);
}

// Input foreground

static const QString inputForegroundKey = QStringLiteral("input/foreground");

QColor Settings::inputForeground() const
{
  const QVariant color = store.value(inputForegroundKey);
  if (color.canConvert<QColor>())
    return color.value<QColor>();
  return Qt::GlobalColor::black;
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

// Output font

static const QString outputFontKey = QStringLiteral("output/font");

QFont Settings::outputFont() const
{
  const QVariant font = store.value(outputFontKey);
  if (font.canConvert<QFont>())
    return font.value<QFont>();

  static const QFont defaultFont = getDefaultFont(12);
  return defaultFont;
}

void Settings::setOutputFont(const QFont &font)
{
  store.setValue(outputFontKey, font);
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
