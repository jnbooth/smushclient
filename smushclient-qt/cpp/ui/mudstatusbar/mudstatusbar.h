#pragma once
#include <QtCore/QByteArray>
#include <QtGui/QContextMenuEvent>
#include <QtWidgets/QMenu>
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

  bool createStat(const QString &entity, const QString &caption, const QString &maxEntity);
  bool updateStat(const QString &entity, const QString &value);

public slots:
  void clearStats();
  void setConnected(bool connected);
  void setMessage(const QString &message);
  void setUsers(int users);
  void setUsers(const QString &users);

protected:
  void contextMenuEvent(QContextMenuEvent *event) override;

private:
  Ui::MudStatusBar *ui;
  QMenu *menu;
  QHash<QString, StatusBarStat *> statsByEntity{};
  QMultiHash<QString, StatusBarStat *> statsByMax{};

private:
  bool recreateStat(StatusBarStat *stat, const QString &caption, const QString &maxEntity);
  bool restore(const QByteArray &data);
  QByteArray save() const;
  static const QString &settingsKey();
  QList<QAction *> stateActions() const;
};
