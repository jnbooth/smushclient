#pragma once
#include <QtCore/QString>

#define PLUGINS_DIR "plugins"

#define SCRIPTS_DIR "lua"

#define SOUNDS_DIR "sounds"

#define WORLDS_DIR "worlds"

QString makePathRelative(const QString &filePath);
QString makePathRelative(const QString &filePath, const QString &relativeTo);
