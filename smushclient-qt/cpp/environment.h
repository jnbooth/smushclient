#pragma once

class QDir;
class QString;

#define LOGS_DIR "logs"

#define PLUGINS_DIR "plugins"

#define SCRIPTS_DIR "lua"

#define SOUNDS_DIR "sounds"

#define WORLDS_DIR "worlds"

QString
defaultStartupDirectory();

bool
initializeStartupDirectory(const QString& dirPath);

QString
makePathRelative(const QString& filePath) noexcept;
QString
makePathRelative(const QString& filePath, const QString& relativeTo) noexcept;

bool
openDirectoryExternally(const QDir& dir);

bool
openDirectoryExternally(const QString& dirPath);
