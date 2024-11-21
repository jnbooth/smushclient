#pragma once
#include <QtCore/QObject>
#include <QtGui/QColor>
#include <QtGui/QFont>
#include <QtGui/QTextBlockFormat>

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
  void outputBlockFormatChanged(const QTextBlockFormat &format);
  void outputFontChanged(const QFont &font);
  void outputHistoryLimitChanged(int limit);
  void outputPaddingChanged(int inset);
  void outputLimitChanged(int limit);
};
