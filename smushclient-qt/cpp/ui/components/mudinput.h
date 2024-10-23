#pragma once
#include <QtWidgets/QLineEdit>
#include "../../commandhistory.h"

class MudInput : public QLineEdit
{
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

protected:
  virtual void keyPressEvent(QKeyEvent *event) override;

private:
  QString draft;
  CommandHistory history;

private:
  void previous();
  void next();
  void restoreDraft();
  void saveDraft();
  void setTextFromHistory(const QString &text);
};
