#include "saveprompt.h"

// Public methods

SavePrompt::SavePrompt(const QString& subject, QWidget* parent)
  : QMessageBox(parent)
{
  setText(tr("Do you want to save the changes you made to %1?").arg(subject));
  setStandardButtons(QMessageBox::Save | QMessageBox::Discard |
                     QMessageBox::Cancel);
  setDefaultButton(QMessageBox::Save);
}
