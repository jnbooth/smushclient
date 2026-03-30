#pragma once
#include <QtCore/QElapsedTimer>
#include <QtWidgets/QMenu>

namespace Ui {
class MudStatusBar;
} // namespace Ui

class StatusBarStat;

class MudStatusBar : public QWidget
{
  Q_OBJECT

public:
  enum class ConnectionStatus
  {
    Disconnected,
    Connected,
    Encrypted,
  };

  explicit MudStatusBar(QWidget* parent = nullptr);
  ~MudStatusBar() override;

  bool createStat(const QString& entity,
                  const QString& caption,
                  const QString& maxEntity);
  bool updateStat(const QString& entity, const QString& value);
  QString message() const;

public slots:
  void clearStats();
  void resetStatusTime();
  void setConnected(ConnectionStatus status);
  void setMessage(const QString& message);
  void setUsers(int users);
  void setUsers(const QString& users);

protected:
  void contextMenuEvent(QContextMenuEvent* event) override;

private:
  bool recreateStat(StatusBarStat* stat,
                    const QString& caption,
                    const QString& maxEntity);
  bool restore();
  void save() const;
  static const QString& settingsKey();
  QList<QAction*> stateActions() const;

private slots:
  void on_action_connection_time_toggled(bool checked);

private:
  Ui::MudStatusBar* ui;
  std::array<QIcon, 3> connectionIcons;
  QMenu* menu;
  QHash<QString, StatusBarStat*> statsByEntity;
  QMultiHash<QString, StatusBarStat*> statsByMax;
  bool connected = false;
};
