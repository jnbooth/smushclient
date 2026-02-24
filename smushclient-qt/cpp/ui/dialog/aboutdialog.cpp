#include "aboutdialog.h"
#include <QtCore/QCoreApplication>

// Public methods

AboutDialog::AboutDialog(QWidget* parent)
  : QMessageBox(parent)
{
  setWindowTitle(tr("About SmushClient"));
  setIconPixmap(QPixmap(QStringLiteral(":/appicon/appicon.svg")));
  setText(QCoreApplication::applicationName());
  setInformativeText(
    tr("Version: %1").arg(QCoreApplication::applicationVersion()));
}
