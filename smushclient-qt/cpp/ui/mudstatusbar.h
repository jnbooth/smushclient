#pragma once
#include <QtWidgets/QWidget>

namespace Ui
{
  class MudStatusBar;
}

class StatusBarStat;

class MudStatusBar : public QWidget
{
  Q_OBJECT

public:
  explicit MudStatusBar(QWidget *parent = nullptr);
  ~MudStatusBar();

  bool createStat(const QString &entity, const QString &caption, const QString &max);
  bool updateStat(const QString &entity, const QString &value);

public slots:
  void clearStats();
  void setConnected(bool connected);
  void setMessage(const QString &message);
  void setUsers(int users);
  void setUsers(const QString &users);

private:
  Ui::MudStatusBar *ui;
  QHash<QString, StatusBarStat *> statsByEntity;
  QMultiHash<QString, StatusBarStat *> statsByMax;
};
