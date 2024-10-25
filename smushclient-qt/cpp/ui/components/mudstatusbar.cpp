#include "mudstatusbar.h"
#include <QtGui/QIcon>
#include <QtWidgets/QHBoxLayout>
#include "iconlabel.h"

// Public methods

MudStatusBar::MudStatusBar(QWidget *parent)
    : QStatusBar(parent)
{
  connectionStatus = new QLabel;
  addPermanentWidget(connectionStatus);

  QIcon networkIcon = QIcon::fromTheme(QIcon::ThemeIcon::NetworkWireless);
  connectionIcon = new IconLabel(networkIcon, QIcon::Mode::Disabled);
  addPermanentWidget(connectionIcon);

  setConnected(false);
}

void MudStatusBar::setConnected(bool connected)
{
  connectionIcon->setMode(connected ? QIcon::Mode::Normal : QIcon::Mode::Disabled);
  connectionStatus->setText(connected ? tr("Connected") : tr("Disconnected"));
}
