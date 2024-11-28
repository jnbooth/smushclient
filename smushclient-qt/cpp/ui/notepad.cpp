#include "notepad.h"
#include "ui_notepad.h"
#include <QtCore/QSaveFile>
#include <QtGui/QDesktopServices>
#include <QtGui/QTextDocumentFragment>
#include <QtGui/QWindow>
#include <QtPrintSupport/QPrintDialog>
#include <QtPrintSupport/QPrinter>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QErrorMessage>
#include "finddialog.h"
#include "settings/settings.h"
#include "../localization.h"
#include "../settings.h"

constexpr int minFontSize = 5;

// Notepad

// Public methods

Notepad::Notepad(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::Notepad)
{
  ui->setupUi(this);
  setAttribute(Qt::WA_DeleteOnClose);
  Settings settings;
  QPalette palette = ui->editor->palette();
  palette.setBrush(QPalette::ColorRole::Text, settings.getNotepadForeground());
  palette.setBrush(QPalette::ColorRole::Base, settings.getNotepadBackground());
  ui->editor->setPalette(palette);
  const QFont font = settings.getNotepadFont();
  fontSize = font.pointSizeF();
  QTextCharFormat format;
  format.setFont(font);
  ui->editor->document()->setDefaultFont(font);
  ui->editor->textCursor().mergeCharFormat(format);
  connect(ui->action_close_window, &QAction::triggered, this, &Notepad::close);
  connect(ui->action_copy, &QAction::triggered, ui->editor, &QTextEdit::copy);
  connect(ui->action_cut, &QAction::triggered, ui->editor, &QTextEdit::cut);
  connect(ui->action_maximize, &QAction::triggered, this, &Notepad::showMaximized);
  connect(ui->action_minimize, &QAction::triggered, this, &Notepad::showMinimized);
  connect(ui->action_paste, &QAction::triggered, ui->editor, &QTextEdit::paste);
  connect(ui->action_restore, &QAction::triggered, this, &Notepad::showNormal);
  connect(ui->action_select_all, &QAction::triggered, ui->editor, &QTextEdit::selectAll);
}

Notepad::~Notepad()
{
  delete ui;
}

QTextEdit *Notepad::editor() const
{
  return ui->editor;
}

// Private methods

void Notepad::applyFontSize()
{
  QTextCharFormat format;
  format.setFontPointSize(fontSize);
  QTextCursor cursor = ui->editor->textCursor();
  cursor.select(QTextCursor::SelectionType::Document);
  cursor.mergeCharFormat(format);
}

// Private slots

void Notepad::on_action_close_all_notepad_windows_triggered()
{
  qobject_cast<Notepads *>(parent())->closeAll();
}

void Notepad::on_action_decrease_size_triggered()
{
  fontSize -= 1;
  if (fontSize <= minFontSize)
    ui->action_decrease_size->setEnabled(false);
  applyFontSize();
}

void Notepad::on_action_find_triggered()
{
  if (!findDialog)
    findDialog = new FindDialog(this);
  if (findDialog->exec() == QDialog::Accepted)
    findDialog->find(ui->editor);
}

void Notepad::on_action_find_again_triggered()
{
  if (findDialog->isFilled() || findDialog->exec() == QDialog::Accepted)
    findDialog->find(ui->editor);
}

void Notepad::on_action_global_preferences_triggered()
{
  Settings settings;
  SettingsDialog(settings, this).exec();
}

void Notepad::on_action_increase_size_triggered()
{
  if (fontSize <= minFontSize)
    ui->action_decrease_size->setEnabled(true);
  fontSize += 1;
  applyFontSize();
}

void Notepad::on_action_print_triggered()
{

  QPrinter printer;
  QPrintDialog dialog(&printer, this);
  if (dialog.exec() != QDialog::Accepted)
    return;
  ui->editor->print(&printer);
}

void Notepad::on_action_reset_size_triggered()
{
  fontSize = ui->editor->document()->defaultFont().pointSizeF();
  ui->action_decrease_size->setEnabled(fontSize > minFontSize);
  applyFontSize();
}

void Notepad::on_action_save_selection_triggered()
{
  const QString path = QFileDialog::getSaveFileName(
      this,
      tr("Save as"),
      QString(),
      FileFilter::text());

  if (path.isEmpty())
    return;

  QSaveFile file(path);
  if (file.open(QSaveFile::WriteOnly))
  {
    file.write(ui->editor->textCursor().selection().toPlainText().toUtf8());
    if (file.commit())
      return;
  }
  QErrorMessage::qtHandler()->showMessage(file.errorString());
  return;
}

void Notepad::on_action_visit_api_guide_triggered()
{
  QDesktopServices::openUrl(QStringLiteral("https://www.gammon.com.au/scripts/doc.php?general=function_list"));
}

void Notepad::on_action_visit_bug_reports_triggered()
{
  QDesktopServices::openUrl(QStringLiteral("https://github.com/jnbooth/smushclient/issues"));
}

// Notepads

// Public methods

Notepads::Notepads(QWidget *parent)
    : QWidget(parent)
{
  hide();
}

void Notepads::closeAll()
{
  for (QObject *child : children())
    delete child;
}

QTextEdit *Notepads::pad(const QString &name)
{
  if (name.isEmpty())
    return create(name)->editor();

  Notepad *notepad = findChild<Notepad *>(name, Qt::FindDirectChildrenOnly);

  if (!notepad)
  {
    notepad = create(name);
    notepad->setWindowTitle(name);
  }

  return notepad->editor();
}

// Private methods

Notepad *Notepads::create(const QString &name)
{
  QWidget *active = QApplication::activeWindow();
  Notepad *notepad = new Notepad(this);
  notepad->setObjectName(name);
  notepad->show();
  if (active)
    active->activateWindow();
  return notepad;
}
