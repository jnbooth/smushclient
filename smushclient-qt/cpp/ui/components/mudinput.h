#pragma once
#include "../../commandhistory.h"
#include <QtWidgets/QTextEdit>

class MudInput : public QTextEdit
{
  Q_OBJECT

public:
  explicit MudInput(QWidget* parent = nullptr);
  MudInput(QWidget* parent, const QStringList& history);
  explicit MudInput(const QStringList& history);

  bool keypadIgnored() const noexcept { return m_keypadIgnored; }
  const QStringList& log() const noexcept;

  QSize minimumSizeHint() const override;
  QSize sizeHint() const override;

public slots:
  void clearLog();
  QString push();
  void remember(const QString& text);
  void setKeypadIgnored(bool ignore = true);
  void setMaxLogSize(int size);

signals:
  void copyTriggered();
  void submitted(const QString& text);

protected:
  void keyPressEvent(QKeyEvent* event) override;

private:
  CommandHistory history;
  bool m_keypadIgnored = false;
};
