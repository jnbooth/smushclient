#pragma once
#include <QtWidgets/QWidget>

namespace Ui {
class PrefsConnecting;
}

class World;

class PrefsConnecting : public QWidget
{
  Q_OBJECT

public:
  explicit PrefsConnecting(const World& world, QWidget* parent = nullptr);
  ~PrefsConnecting();

private:
  Ui::PrefsConnecting* ui;
};
