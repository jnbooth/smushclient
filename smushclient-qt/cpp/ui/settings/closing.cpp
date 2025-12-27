#include "closing.h"
#include "../../enumbuttongroup.h"
#include "../../fieldconnector.h"
#include "../../settings.h"
#include "ui_closing.h"

SettingsClosing::SettingsClosing(Settings& settings, QWidget* parent)
  : QWidget(parent)
  , ui(new Ui::SettingsClosing)
{
  ui->setupUi(this);

  if (settings.getConfirmQuit())
    ui->ConfirmQuit->toggle();
  else
    ui->ConfirmQuit_false->toggle();
  connect(ui->ConfirmQuit,
          &QRadioButton::toggled,
          &settings,
          &Settings::setConfirmQuit);

  EnumButtonGroup(this,
                  settings.getWorldCloseBehavior(),
                  &settings,
                  &Settings::setWorldCloseBehavior)
    .addButton(ui->WorldCloseBehavior_Save, Settings::WorldCloseBehavior::Save)
    .addButton(ui->WorldCloseBehavior_Confirm,
               Settings::WorldCloseBehavior::Confirm)
    .addButton(ui->WorldCloseBehavior_Discard,
               Settings::WorldCloseBehavior::Discard);
}

SettingsClosing::~SettingsClosing()
{
  delete ui;
}
