#pragma once
#include <QtCore/QDir>
#include <QtCore/QString>

#define LOGS_DIR "logs"

#define PLUGINS_DIR "plugins"

#define SCRIPTS_DIR "lua"

#define SOUNDS_DIR "sounds"

#define WORLDS_DIR "worlds"

QString defaultStartupDirectory();

bool initializeStartupDirectory(const QString &dirPath);

QString makePathRelative(const QString &filePath);
QString makePathRelative(const QString &filePath, const QString &relativeTo);

bool openDirectoryExternally(const QString &dirPath);

inline bool openDirectoryExternally(const QDir &dir) {
  return openDirectoryExternally(dir.absolutePath());
}
