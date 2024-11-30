#include "statusbarstat.h"
#include "ui_statusbarstat.h"

// Private constructor

StatusBarStat::StatusBarStat(QWidget *parent, const QString &maxEntity)
    : QWidget(parent),
      ui(new Ui::StatusBarStat),
      maxEntityName(maxEntity)
{
  ui->setupUi(this);
  menuAction = new QAction(this);
  menuAction->setCheckable(true);
  menuAction->setChecked(true);
  menuAction->setVisible(false);
  setVisible(false);
  connect(menuAction, &QAction::toggled, this, &StatusBarStat::setVisible);
}

// Public methods

StatusBarStat::StatusBarStat(QWidget *parent)
    : StatusBarStat(parent, QString()) {}

StatusBarStat::StatusBarStat(const QString &caption, const QString &maxEntity, QWidget *parent)
    : StatusBarStat(parent, maxEntity)
{
  setCaption(caption);
}

StatusBarStat::StatusBarStat(const QString &caption, QWidget *parent)
    : StatusBarStat(caption, QString(), parent) {}

StatusBarStat::~StatusBarStat()
{
  delete ui;
}

// Public slots

void StatusBarStat::setCaption(const QString &caption)
{
  QString formattedCaption = caption.toLower() == caption ? caption.toUpper() : caption;
  menuAction->setText(formattedCaption);
  formattedCaption.push_back(QStringLiteral(": "));
  ui->caption->setText(formattedCaption);
}

void StatusBarStat::setMax(const QString &max)
{
  if (max.isEmpty())
  {
    ui->max->hide();
    ui->value->setMinimumWidth(0);
  }
  else
  {
    ui->max->setText(QStringLiteral("/") + max);
    ui->max->setVisible(true);
  }
}

void StatusBarStat::setMaxEntity(const QString &maxEntity)
{
  maxEntityName = maxEntity;
}

void StatusBarStat::setValue(const QString &value)
{
  ui->value->setText(value);
}

// Protected overries

void StatusBarStat::resizeEvent(QResizeEvent *)
{
  if (ui->max->isVisible())
    ui->value->setMinimumWidth(ui->max->width());
}
