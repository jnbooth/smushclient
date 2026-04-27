#include "environment.h"
#include <QtCore/QDir>
#include <QtCore/QProcess>
#include <QtCore/QStandardPaths>
extern "C"
{
#include "lua.h"
}

using std::string_view;

using Qt::StringLiterals::operator""_L1;

#if defined(Q_OS_WIN)
#define DIR_SEP "\\"
#else
#define DIR_SEP "/"
#endif

constexpr const string_view luaPathSuffix =
  DIR_SEP SCRIPTS_DIR DIR_SEP "?.lua;" LUA_PATH_DEFAULT;

QString
defaultStartupDirectory()
{
#if defined(STANDALONE_CLIENT)
  static const QString dir = QDir::currentPath();
#else
  static const QString dir =
    QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).first();
#endif
  return dir;
}

bool
initializeStartupDirectory(const QString& dirPath)
{
  const QDir appDir(dirPath);
  const bool success =
    appDir.mkpath(LOGS_DIR ""_L1) && appDir.mkpath(PLUGINS_DIR ""_L1) &&
    appDir.mkpath(SCRIPTS_DIR ""_L1) && appDir.mkpath(SOUNDS_DIR ""_L1) &&
    appDir.mkpath(WORLDS_DIR ""_L1);
  if (!success) {
    return false;
  }
  std::string luaPath = dirPath.toStdString();
  luaPath.append(luaPathSuffix);
#if defined(Q_OS_WINDOWS)
  _putenv_s("LUA_PATH", luaPath.c_str());
#else
  setenv("LUA_PATH", luaPath.c_str(), 0);
#endif
  return true;
}

QString
makePathRelative(const QString& filePath) noexcept
{
  return makePathRelative(filePath, QDir::currentPath());
}

QString
makePathRelative(const QString& filePath, const QString& relativeTo) noexcept
{
  if (!filePath.startsWith(relativeTo)) {
    return filePath;
  }
  const qsizetype relativeToSize = relativeTo.size() + 1;
  return filePath.sliced(relativeToSize, filePath.size() - relativeToSize);
}

bool
openDirectoryExternally(const QDir& dir)
{
  return openDirectoryExternally(dir.absolutePath());
}

bool
openDirectoryExternally(const QString& dirPath)
{
#if defined(Q_OS_LINUX)
  return QProcess::execute("xdg-open"_L1, { dirPath }) == 0;
#elif defined(Q_OS_MACOS)
  return QProcess::execute("open"_L1, { dirPath }) == 0;
#elif defined(Q_OS_WIN)
  return QProcess::startDetached("explorer"_L1, { dirPath }) == 0;
#endif
}
