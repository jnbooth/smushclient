#pragma once
#include <QtCore/QByteArray>
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
  bool restore(const QByteArray &data);
  QByteArray save() const;
  bool updateStat(const QString &entity, const QString &value);

public slots:
  void clearStats();
  void setConnected(bool connected);
  void setMessage(const QString &message);
  void setUsers(int users);
  void setUsers(const QString &users);

private:
  Ui::MudStatusBar *ui;
  QSet<QString> hiddenEntities{};
  QHash<QString, StatusBarStat *> statsByEntity{};
  QMultiHash<QString, StatusBarStat *> statsByMax{};
};
