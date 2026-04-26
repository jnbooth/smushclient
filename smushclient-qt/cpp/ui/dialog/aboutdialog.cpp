#include "aboutdialog.h"
#include <QtCore/QCoreApplication>

using Qt::StringLiterals::operator""_L1;

// Public methods

AboutDialog::AboutDialog(QWidget* parent)
  : QMessageBox(parent)
{
  setWindowTitle(tr("About SmushClient"));
  setIconPixmap(QPixmap(":/appicon/appicon.svg"_L1));
  setText(QCoreApplication::applicationName());
  setInformativeText(
    tr("Version: %1").arg(QCoreApplication::applicationVersion()));
}
