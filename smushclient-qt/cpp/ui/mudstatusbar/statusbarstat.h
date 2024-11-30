#pragma once
#include <QtGui/QResizeEvent>
#include <QtWidgets/QFrame>
#include <QtWidgets/QMenu>

namespace Ui
{
  class StatusBarStat;
}

class StatusBarStat : public QWidget
{
  Q_OBJECT

public:
  StatusBarStat(const QString &entity, const QString &caption, const QString &maxEntity, QWidget *parent = nullptr);
  ~StatusBarStat();

  bool isToggled() const;
  constexpr QMenu *menu() const noexcept
  {
    return displayMenu;
  }
  constexpr const QString &entity() const noexcept
  {
    return entityName;
  }
  constexpr const QString &maxEntity() const noexcept
  {
    return maxEntityName;
  }
  void setMaxEntity(const QString &maxEntity);
  void setToggled(bool toggled);

public slots:
  void setCaption(const QString &caption);
  void setMax(const QString &max);
  void setValue(const QString &value);

protected:
  void resizeEvent(QResizeEvent *event) override;

private slots:
  void on_action_reset_colors_triggered();
  void on_action_set_caption_color_triggered();
  void on_action_set_color_triggered();
  void on_action_set_value_color_triggered();

private:
  Ui::StatusBarStat *ui;
  QString entityName;
  QString maxEntityName;
  QMenu *displayMenu;

private:
  QPalette chooseColor(const QWidget *source);
  bool restore(const QByteArray &data);
  QByteArray save() const;
  QString settingsKey() const;
};
