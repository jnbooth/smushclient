#include "scripting.h"
#include "ui_scripting.h"
#include <QtGui/QDesktopServices>
#include <QtWidgets/QErrorMessage>
#include <QtWidgets/QFileDialog>
#include "../../environment.h"
#include "../../fieldconnector.h"

constexpr const char *fileFilter = "Lua files (*.lua);;All Files (*.*)";

// Public methods

PrefsScripting::PrefsScripting(World &world, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::PrefsScripting),
      world(world)
{
  ui->setupUi(this);
  CONNECT_WORLD(EnableScripts);
  CONNECT_WORLD(WorldScript);
  CONNECT_WORLD(ScriptReloadOption);
  CONNECT_WORLD(NoteTextColour);
  CONNECT_WORLD(NoteBackgroundColour);
  CONNECT_WORLD(ErrorTextColour);
  CONNECT_WORLD(ErrorBackgroundColour);
}

PrefsScripting::~PrefsScripting()
{
  delete ui;
}

// Private methods

QString PrefsScripting::defaultScriptPath() const
{
  return QStringLiteral(WORLDS_DIR "/%1").arg(world.getName());
}

// Private slots

void PrefsScripting::on_WorldScript_browse_clicked()
{
  const QString path = QFileDialog::getOpenFileName(
      this,
      tr("Select world script"),
      QStringLiteral(WORLDS_DIR),
      tr(fileFilter));

  if (path.isEmpty())
    return;

  ui->WorldScript->setText(path);
}

void PrefsScripting::on_WorldScript_create_clicked()
{
  const QString path = QFileDialog::getSaveFileName(
      this,
      tr("Save as"),
      defaultScriptPath(),
      tr(fileFilter));

  if (path.isEmpty())
    return;

  QFile file(path);
  if (!file.open(QIODevice::WriteOnly))
  {
    QErrorMessage::qtHandler()->showMessage(file.errorString());
    return;
  }
  file.close();
  ui->WorldScript->setText(path);
}

void PrefsScripting::on_WorldScript_edit_clicked()
{
  const QString &scriptPath = ui->WorldScript->text();
  if (QDesktopServices::openUrl(QUrl::fromLocalFile(scriptPath)))
    return;

  const QString path = QFileDialog::getSaveFileName(
      this,
      tr("Create world script"),
      scriptPath.isEmpty() ? defaultScriptPath() : scriptPath,
      tr(fileFilter));

  if (path.isEmpty())
  {
    ui->WorldScript->setText(path);
    return;
  }

  QFile file(path);
  if (!file.open(QIODevice::WriteOnly))
  {
    QErrorMessage::qtHandler()->showMessage(file.errorString());
    ui->WorldScript->setText(QString());
    return;
  }
  file.close();
  ui->WorldScript->setText(path);
  QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void PrefsScripting::on_WorldScript_textChanged(const QString &value)
{
  if (value.isEmpty())
  {
    ui->WorldScript_edit->setEnabled(false);
    ui->fileNotFound->setVisible(false);
    return;
  }

  if (QFileInfo(value).isReadable())
  {
    ui->WorldScript_edit->setEnabled(true);
    ui->fileNotFound->setVisible(false);
    return;
  }

  ui->WorldScript_edit->setEnabled(false);
  ui->fileNotFound->setVisible(true);
}
