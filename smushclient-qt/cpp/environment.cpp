#include "environment.h"
#include <QtCore/QDir>
#include <QtCore/QProcess>
#include <QtCore/QStandardPaths>

QString defaultStartupDirectory()
{
#if defined(STANDALONE_CLIENT)
  const static QString dir = QDir::currentPath();
#else
  const static QString dir = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).first();
#endif
  return dir;
}

bool initializeStartupDirectory(const QString &dirPath)
{
  const QDir appDir(dirPath);
  return appDir.mkpath(QStringLiteral(LOGS_DIR)) &&
         appDir.mkpath(QStringLiteral(PLUGINS_DIR)) &&
         appDir.mkpath(QStringLiteral(SCRIPTS_DIR)) &&
         appDir.mkpath(QStringLiteral(SOUNDS_DIR)) &&
         appDir.mkpath(QStringLiteral(WORLDS_DIR));
}

QString makePathRelative(const QString &filePath)
{
  return makePathRelative(filePath, QDir::currentPath());
}

QString makePathRelative(const QString &filePath, const QString &relativeTo)
{
  if (!filePath.startsWith(relativeTo))
    return filePath;
  const qsizetype relativeToSize = relativeTo.size() + 1;
  return filePath.sliced(relativeToSize, filePath.size() - relativeToSize);
}

bool openDirectoryExternally(const QString &dirPath)
{
#if defined(Q_OS_LINUX)
  return QProcess::execute(QStringLiteral("xdg-open"), {dirPath});
#elif defined(Q_OS_MACOS)
  return QProcess::execute(QStringLiteral("open"), {dirPath});
#elif defined(Q_OS_WIN)
  return QProcess::startDetached(QStringLiteral("explorer"), {dirPath});
#endif
}
