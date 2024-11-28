#pragma once
#include <QtWidgets/QFrame>
#include <QtGui/QResizeEvent>

namespace Ui
{
  class StatusBarStat;
}

class StatusBarStat : public QWidget
{
  Q_OBJECT

public:
  explicit StatusBarStat(QWidget *parent = nullptr);
  explicit StatusBarStat(const QString &caption, QWidget *parent = nullptr);
  StatusBarStat(const QString &caption, const QString &maxEntity, QWidget *parent = nullptr);
  ~StatusBarStat();

  constexpr const QString &maxEntity() const
  {
    return maxEntityName;
  }
  void setMaxEntity(const QString &maxEntity);

public slots:
  void setCaption(const QString &caption);
  void setMax(const QString &max);
  void setValue(const QString &value);

protected:
  void resizeEvent(QResizeEvent *event) override;

private:
  StatusBarStat(QWidget *parent, const QString &maxEntity);
  Ui::StatusBarStat *ui;
  QString maxEntityName;
};
