#pragma once
#include <QtWidgets/QLabel>
#include <QtWidgets/QStatusBar>

class IconLabel;

class MudStatusBar : public QStatusBar
{
  Q_OBJECT

public:
  explicit MudStatusBar(QWidget *parent = nullptr);
  virtual ~MudStatusBar() {};

  void setConnected(bool connected);

private:
  IconLabel *connectionIcon;
  QLabel *connectionStatus;
};
