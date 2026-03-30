#include "styledialog.h"
#include "../../spans.h"
#include "ui_styledialog.h"

// Private utils

namespace {
QColor
brushColor(const QBrush& brush)
{
  if (brush.style() == Qt::BrushStyle::NoBrush) {
    return QColor();
  }
  const QColor color = brush.color();
  if (color.alpha() == 0) {
    return QColor();
  }
  return color;
}
} // namespace

// Public methods

StyleDialog::StyleDialog(const QTextCharFormat& format, QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::StyleDialog)
{
  ui->setupUi(this);

  const QColor foreground = brushColor(format.foreground());
  if (foreground.isValid()) {
    ui->foreground_color->setValue(foreground);
    ui->foreground_text->setText(foreground.name());
  } else {
    ui->foreground_color->hide();
  }

  const QColor background = brushColor(format.background());
  if (background.isValid()) {
    ui->background_color->setValue(background);
    ui->background_text->setText(background.name());
  } else {
    ui->background_color->hide();
  }

  const TextStyles styles = spans::getStyles(format);

#define FLAG(field) ui->field->setVisible(styles.testFlag(TextStyle::field));
#define FLAG_OR(field, pred)                                                   \
  ui->field->setVisible((pred) || styles.testFlag(TextStyle::field));

  FLAG(NonProportional);
  FLAG_OR(Bold, format.fontWeight() >= QFont::Weight::Bold);
  FLAG(Faint);
  FLAG_OR(Italic, format.fontItalic());
  FLAG_OR(Underline, format.fontUnderline());
  FLAG(Blink);
  FLAG(Small);
  FLAG(Inverse);
  FLAG(Conceal);
  FLAG_OR(Strikeout, format.fontStrikeOut());
  FLAG(Highlight);
}

StyleDialog::~StyleDialog()
{
  delete ui;
}
