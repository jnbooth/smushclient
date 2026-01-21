#pragma once
#include <QtGui/QTextBlockFormat>

class SettingsNotifier : public QObject
{
  Q_OBJECT

public:
  explicit SettingsNotifier(QObject* parent = nullptr)
    : QObject(parent)
  {
  }

signals:
  void backgroundMaterialChanged(std::optional<int> material);
  void inputBackgroundChanged(const QColor& color);
  void inputForegroundChanged(const QColor& color);
  void inputFontChanged(const QFont& font);
  void inputHistoryLimitChanged(int limit);
  void outputBlockFormatChanged(const QTextBlockFormat& format);
  void outputFontChanged(const QFont& font);
  void outputHistoryLimitChanged(int limit);
  void outputPaddingChanged(qreal inset);
  void outputLimitChanged(int limit);
};
