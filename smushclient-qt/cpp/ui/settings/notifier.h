#pragma once
#include <QtCore/QObject>
#include <QtGui/QColor>
#include <QtGui/QFont>

class SettingsNotifier : public QObject
{
  Q_OBJECT

public:
  explicit inline SettingsNotifier(QObject *parent = nullptr)
      : QObject(parent) {}

signals:
  void inputBackgroundChanged(const QColor &color);
  void inputForegroundChanged(const QColor &color);
  void inputFontChanged(const QFont &font);
  void inputHistoryLimitChanged(int limit);
  void outputFontChanged(const QFont &font);
  void outputHistoryLimitChanged(int limit);
  void outputLimitChanged(int limit);
};
