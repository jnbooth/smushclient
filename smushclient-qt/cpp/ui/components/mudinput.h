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
  virtual ~MudInput() {};

  const QStringList& log() const noexcept;
  void setLog(const QStringList& log);

  virtual QSize minimumSizeHint() const override;
  virtual QSize sizeHint() const override;

public slots:
  void clearLog();
  void remember(const QString& text);
  void setIgnoreKeypad(bool ignore);
  void setMaxLogSize(int size);

signals:
  void submitted(const QString& text);

protected:
  virtual void keyPressEvent(QKeyEvent* event) override;

private:
  CommandHistory history;
  bool ignoreKeypad = false;
};
