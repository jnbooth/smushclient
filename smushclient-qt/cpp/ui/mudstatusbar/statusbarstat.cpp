#include "statusbarstat.h"
#include "ui_statusbarstat.h"
#include <QtCore/QSettings>
#include <QtWidgets/QColorDialog>

// Private utils

namespace {
QColor
saveColor(QWidget* widget, const QColor& baseColor)
{
  const QColor color = widget->palette().color(QPalette::ColorRole::WindowText);
  return baseColor == color ? QColor() : color;
}

void
setTextColor(QWidget* widget, const QColor& color)
{
  QPalette palette;
  if (color.isValid()) {
    palette.setColor(QPalette::ColorRole::WindowText, color);
  }
  widget->setPalette(palette);
}
} // namespace

// Public methods

StatusBarStat::StatusBarStat(const QString& entity,
                             const QString& caption,
                             const QString& maxEntity,
                             QWidget* parent)
  : QWidget(parent)
  , ui(new Ui::StatusBarStat)
  , m_entity(entity)
  , m_maxEntity(maxEntity)
  , displayMenu(new QMenu(this))
{
  ui->setupUi(this);
  setVisible(false);

  displayMenu->addAction(ui->action_display);
  displayMenu->addAction(ui->action_show_max);
  displayMenu->addAction(ui->action_set_color);
  displayMenu->addAction(ui->action_set_caption_color);
  displayMenu->addAction(ui->action_set_value_color);
  displayMenu->addAction(ui->action_reset_colors);
  setCaption(caption);
  restore();
}

StatusBarStat::~StatusBarStat()
{
  save();
  delete ui;
}

// Public slots

bool
StatusBarStat::toggled() const
{
  return ui->action_display->isChecked();
}

void
StatusBarStat::setCaption(const QString& caption)
{
  QString formattedCaption =
    caption.toLower() == caption ? caption.toUpper() : caption;
  displayMenu->setTitle(formattedCaption);
  formattedCaption.push_back(QStringLiteral(": "));
  ui->caption->setText(formattedCaption);
}

void
StatusBarStat::setMax(const QString& max)
{
  const bool hasMax = !max.isEmpty();
  ui->action_show_max->setVisible(hasMax);
  ui->max->setText(hasMax ? QStringLiteral("/") + max : QString());

  const bool showMax = hasMax && ui->action_show_max->isChecked();
  ui->max->setVisible(showMax);
  if (!showMax) {
    ui->value->setMinimumWidth(0);
  }
}

void
StatusBarStat::setMaxEntity(const QString& maxEntity)
{
  m_maxEntity = maxEntity;
}

void
StatusBarStat::setToggled(bool toggled)
{
  ui->action_display->setChecked(toggled);
}

void
StatusBarStat::setValue(const QString& value)
{
  ui->value->setText(value);
  if (value.isEmpty()) {
    displayMenu->menuAction()->setVisible(false);
    setVisible(false);
  } else {
    displayMenu->menuAction()->setVisible(true);
    setVisible(ui->action_display->isChecked());
  }
}

// Protected overrides

void
StatusBarStat::resizeEvent(QResizeEvent* /*event*/)
{
  if (ui->max->isVisible()) {
    ui->value->setMinimumWidth(ui->max->width());
  }
}

// Private methods

QPalette
StatusBarStat::chooseColor(const QWidget* source)
{
  QPalette colorPalette = source->palette();
  const QColor color =
    QColorDialog::getColor(colorPalette.color(QPalette::ColorRole::WindowText),
                           this,
                           displayMenu->title(),
                           QColorDialog::ColorDialogOption::ShowAlphaChannel);

  if (!color.isValid()) {
    return colorPalette;
  }

  colorPalette.setColor(QPalette::ColorRole::WindowText, color);
  return colorPalette;
}

bool
StatusBarStat::restore()
{
  const QByteArray saveData = QSettings().value(settingsKey()).toByteArray();
  if (saveData.isEmpty()) {
    return false;
  }

  bool display;
  bool showMax;
  QColor captionColor;
  QColor valueColor;

  QDataStream stream(saveData);
  stream >> display >> showMax >> captionColor >> valueColor;

  ui->action_display->setChecked(display);
  ui->action_show_max->setChecked(showMax);
  setTextColor(ui->caption, captionColor);
  setTextColor(ui->value, valueColor);
  setTextColor(ui->max, valueColor);

  return stream.status() == QDataStream::Status::Ok;
}

void
StatusBarStat::save() const
{
  const QColor baseColor = palette().color(QPalette::ColorRole::WindowText);
  QByteArray saveData;
  QDataStream stream(&saveData, QIODevice::WriteOnly);
  stream << ui->action_display->isChecked() << ui->action_show_max->isChecked()
         << saveColor(ui->caption, baseColor)
         << saveColor(ui->value, baseColor);
  QSettings().setValue(settingsKey(), saveData);
}

QString
StatusBarStat::settingsKey() const
{
  return QStringLiteral("state/stat/") + entity();
}

// Private slots

void
StatusBarStat::on_action_reset_colors_triggered()
{
  ui->caption->setPalette(QPalette());
  ui->value->setPalette(QPalette());
  ui->max->setPalette(QPalette());
}

void
StatusBarStat::on_action_set_caption_color_triggered()
{
  const QPalette palette = chooseColor(ui->caption);
  ui->caption->setPalette(palette);
}

void
StatusBarStat::on_action_set_color_triggered()
{
  const QPalette palette = chooseColor(ui->value);
  ui->caption->setPalette(palette);
  ui->value->setPalette(palette);
  ui->max->setPalette(palette);
}

void
StatusBarStat::on_action_set_value_color_triggered()
{
  const QPalette palette = chooseColor(ui->value);
  ui->value->setPalette(palette);
  ui->max->setPalette(palette);
}
