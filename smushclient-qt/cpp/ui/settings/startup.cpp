#include "startup.h"
#include "ui_startup.h"
#include <QtWidgets/QFileDialog>
#include "../../environment.h"
#include "../../enumbuttongroup.h"
#include "../../localization.h"
#include "../../settings.h"

SettingsStartup::SettingsStartup(Settings &settings, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::SettingsStartup),
      settings(settings)
{
  ui->setupUi(this);

  for (const QString &file : settings.getOpenAtStartup())
    ui->OpenAtStartup->addItem(file);

  EnumButtonGroup(this, settings.getStartupBehavior(), &settings, &Settings::setStartupBehavior)
      .addButton(ui->StartupBehavior_None, Settings::StartupBehavior::None)
      .addButton(ui->StartupBehavior_Reopen, Settings::StartupBehavior::Reopen)
      .addButton(ui->StartupBehavior_List, Settings::StartupBehavior::List);
}

SettingsStartup::~SettingsStartup()
{
  const int count = ui->OpenAtStartup->count();
  QStringList openAtStartup;
  openAtStartup.reserve(count);
  for (int i = 0; i < count; ++i)
    openAtStartup.push_back(ui->OpenAtStartup->item(i)->text());
  settings.setOpenAtStartup(openAtStartup);
  delete ui;
}

void SettingsStartup::on_OpenAtStartup_add_clicked()
{
  const QStringList filePaths = QFileDialog::getOpenFileNames(
      this,
      tr("Add world"),
      QStringLiteral(WORLDS_DIR),
      FileFilter::world());

  if (filePaths.isEmpty())
    return;

  for (const QString &filePath : filePaths)
    ui->OpenAtStartup->addItem(makePathRelative(filePath));
}

void SettingsStartup::on_OpenAtStartup_remove_clicked()
{
  for (QListWidgetItem *item : ui->OpenAtStartup->selectedItems())
    delete item;
}

void SettingsStartup::on_OpenAtStartup_up_clicked()
{
  const QModelIndex selected = ui->OpenAtStartup->currentIndex();
  const QModelIndex parent = selected.parent();
  const int row = selected.row();
  const int position = row - 1;
  ui->OpenAtStartup->model()->moveRows(parent, row, 1, parent, position);
  const int count = ui->OpenAtStartup->count();
  ui->OpenAtStartup_down->setEnabled(position != count - 1);
  ui->OpenAtStartup_up->setEnabled(position != 0 && count > 1);
}

void SettingsStartup::on_OpenAtStartup_down_clicked()
{
  const QModelIndex selected = ui->OpenAtStartup->currentIndex();
  const QModelIndex parent = selected.parent();
  const int row = selected.row();
  const int position = row + 1;
  ui->OpenAtStartup->model()->moveRows(parent, position, 1, parent, row);
  const int count = ui->OpenAtStartup->count();
  ui->OpenAtStartup_down->setEnabled(position != count - 1);
  ui->OpenAtStartup_up->setEnabled(position != 0 && count > 1);
}

void SettingsStartup::on_OpenAtStartup_itemSelectionChanged()
{
  const QModelIndexList indexes = ui->OpenAtStartup->selectionModel()->selectedIndexes();
  const qsizetype selectionSize = indexes.size();
  ui->OpenAtStartup_remove->setEnabled(selectionSize);
  if (selectionSize != 1)
  {
    ui->OpenAtStartup_down->setEnabled(false);
    ui->OpenAtStartup_up->setEnabled(false);
    return;
  }
  const int position = indexes.first().row();
  const int count = ui->OpenAtStartup->count();
  ui->OpenAtStartup_down->setEnabled(position != count - 1);
  ui->OpenAtStartup_up->setEnabled(position != 0 && count > 1);
}
