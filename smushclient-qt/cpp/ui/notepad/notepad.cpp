#include "notepad.h"
#include "../../config.h"
#include "../../environment.h"
#include "../../localization.h"
#include "../../settings.h"
#include "../dialog/aboutdialog.h"
#include "../dialog/finddialog.h"
#include "../dialog/saveprompt.h"
#include "../settings/settings.h"
#include "notepads.h"
#include "ui_notepad.h"
#include <QtCore/QSaveFile>
#include <QtGui/QCloseEvent>
#include <QtGui/QDesktopServices>
#include <QtGui/QTextDocumentFragment>
#include <QtGui/QWindow>
#include <QtPrintSupport/QPrintDialog>
#include <QtPrintSupport/QPrinter>
#include <QtWidgets/QErrorMessage>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QFontDialog>
#include <QtWidgets/QMessageBox>

// Notepad

// Public methods

Notepad::Notepad(const QString& name, QWidget* parent)
  : Notepad(QUuid(), name, parent)
{
}

Notepad::Notepad(QUuid worldID, const QString& name, QWidget* parent)
  : QMainWindow(parent)
  , ui(new Ui::Notepad)
  , worldID(worldID)
{
  ui->setupUi(this);
  setObjectName(name);
  setAttribute(Qt::WA_DeleteOnClose);
  Settings settings;
  ui->editor->setPalette(settings.getNotepadPalette());
  const QFont font = settings.getNotepadFont();
  defaultFontSize = font.pointSizeF();
  ui->editor->setFont(font);
  connect(ui->editor->document(),
          &QTextDocument::modificationChanged,
          this,
          &QWidget::setWindowModified);
  connect(ui->action_save, &QAction::triggered, this, &Notepad::save);
  connect(ui->action_save_as, &QAction::triggered, this, &Notepad::saveAs);
  updateWindowTitle(name.isEmpty() ? windowTitle() : name);
}

Notepad::~Notepad()
{
  delete ui;
}

QTextDocument*
Notepad::document() const
{
  return ui->editor->document();
}

QTextEdit*
Notepad::editor() const
{
  return ui->editor;
}

// Public slots

void
Notepad::appendText(const QString& text)
{
  QTextCursor cursor = ui->editor->textCursor();
  if (!cursor.atBlockStart()) {
    cursor.insertBlock();
  }
  cursor.insertText(text);
}

QString
Notepad::save()
{
  if (filePath.isEmpty()) {
    return saveAs();
  }
  if (!writeToFile(filePath)) {
    return QString();
  }
  return filePath;
}

QString
Notepad::saveAsNew(const QString& path, bool separate)
{
  const QString userPath =
    path.isEmpty() ? QFileDialog::getSaveFileName(
                       this, tr("Save as"), fileHint(), FileFilter::text())
                   : path;

  if (userPath.isEmpty() || !writeToFile(userPath)) {
    return QString();
  }
  if (!separate) {
    filePath = makePathRelative(userPath);
    updateWindowTitle(QFileInfo(userPath).fileName());
  }
  return filePath;
}

void
Notepad::setSaveMethod(SaveMethod method)
{
  promptBeforeClose = method != SaveMethod::NeverPrompt;
}

void
Notepad::setText(const QString& text)
{
  ui->editor->setPlainText(text);
}

// Protected overrides

void
Notepad::closeEvent(QCloseEvent* event)
{
  if (promptBeforeClose && isWindowModified() && !promptSave()) {
    event->ignore();
    return;
  }
  event->accept();
}

// Private methods

bool
Notepad::promptSave()
{
  SavePrompt prompt(windowTitleBase, this);
  switch (prompt.exec()) {
    case QMessageBox::Save:
      return !save().isEmpty();
    case QMessageBox::Discard:
      return true;
    case QMessageBox::Cancel:
      return false;
    default:
      return true;
  }
}

void
Notepad::updateWindowTitle(const QString& name)
{
  windowTitleBase = name;
  const QString appName = QCoreApplication::applicationName();
  setWindowTitle(isLeftToRight() ? name + QStringLiteral("[*] - ") + appName
                                 : appName + QStringLiteral(" - [*]") + name);
}

QString
Notepad::fileHint() const
{
  QString name = objectName();
  if (!name.isEmpty()) {
    name.append(QStringLiteral(".txt"));
  }
  return name;
}

bool
Notepad::writeToFile(const QString& path)
{
  QFile file(path);
  if (!file.open(QSaveFile::WriteOnly)) {
    QErrorMessage::qtHandler()->showMessage(file.errorString());
    return false;
  }
  file.write(ui->editor->toPlainText().toUtf8());
  file.close();
  setWindowModified(false);
  return true;
}

// Private slots

void
Notepad::on_action_about_triggered()
{
  AboutDialog(this).exec();
}

void
Notepad::on_action_close_all_notepad_windows_triggered()
{
  qobject_cast<Notepads*>(parent())->closeAll();
}

void
Notepad::on_action_decrease_size_triggered()
{
  ui->editor->zoomOut();
}

void
Notepad::on_action_find_triggered()
{
  if (findDialog == nullptr) {
    findDialog = new FindDialog(this);
  }
  if (findDialog->exec() == QDialog::Accepted) {
    findDialog->find(ui->editor);
  }
}

void
Notepad::on_action_find_again_triggered()
{
  if (findDialog->filled() || findDialog->exec() == QDialog::Accepted) {
    findDialog->find(ui->editor);
  }
}

void
Notepad::on_action_font_triggered()
{
  bool ok;
  const QFont font = QFontDialog::getFont(&ok, ui->editor->font(), this);
  if (!ok) {
    return;
  }
  defaultFontSize = font.pointSizeF();
  ui->editor->setFont(font);
}

void
Notepad::on_action_global_preferences_triggered()
{
  Settings settings;
  SettingsDialog(settings, this).exec();
}

void
Notepad::on_action_increase_size_triggered()
{
  ui->editor->zoomIn();
}

void
Notepad::on_action_print_triggered()
{
  QPrinter printer;
  QPrintDialog dialog(&printer, this);
  if (dialog.exec() != QDialog::Accepted) {
    return;
  }
  ui->editor->print(&printer);
}

void
Notepad::on_action_reset_size_triggered()
{
  QFont f = ui->editor->font();
  f.setPointSizeF(2);
  ui->editor->setFont(f);
}

void
Notepad::on_action_save_selection_triggered()
{
  const QString path = QFileDialog::getSaveFileName(
    this, tr("Save as"), QString(), FileFilter::text());

  if (path.isEmpty()) {
    return;
  }

  QSaveFile file(path);
  if (file.open(QSaveFile::WriteOnly)) {
    file.write(ui->editor->textCursor().selection().toPlainText().toUtf8());
    if (file.commit()) {
      return;
    }
  }
  QErrorMessage::qtHandler()->showMessage(file.errorString());
}

void
Notepad::on_action_visit_api_guide_triggered()
{
  QDesktopServices::openUrl(config::apiGuide());
}

void
Notepad::on_action_visit_bug_reports_triggered()
{
  QDesktopServices::openUrl(config::issues());
}
