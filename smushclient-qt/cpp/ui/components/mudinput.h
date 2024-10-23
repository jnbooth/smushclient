#pragma once
#include <QtWidgets/QTextEdit>
#include "../../commandhistory.h"

class MudInput : public QTextEdit
{
  Q_OBJECT

public:
  explicit MudInput(QWidget *parent = nullptr);
  MudInput(QWidget *parent, const QStringList &history);
  explicit MudInput(const QStringList &history);
  virtual ~MudInput() {};

  void clearLog();
  void forgetLast() noexcept;
  const QStringList &log() const noexcept;
  void remember(const QString &text);
  void setLog(const QStringList &log);
  void setMaxLogSize(qsizetype size);

  virtual QSize minimumSizeHint() const override;
  virtual QSize sizeHint() const override;

signals:
  void submitted(const QString &text);

protected:
  virtual void keyPressEvent(QKeyEvent *event) override;

private:
  QString draft;
  CommandHistory history;

private:
  void restoreDraft();
  void saveDraft();
  void setTextFromHistory(const QString &text);
};
