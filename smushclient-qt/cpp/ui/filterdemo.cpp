#include "filterdemo.h"
#include "../localization.h"
#include "../scripting/miniwindow/imagefilters.h"
#include "ui_filterdemo.h"

#include <QtWidgets/QErrorMessage>
#include <QtWidgets/QFileDialog>

// Public methods

FilterDemo::FilterDemo(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::FilterDemo)
{
  ui->setupUi(this);
  base = ui->image->pixmap();
  on_filter_currentIndexChanged(ui->filter->currentIndex());
}

FilterDemo::~FilterDemo()
{
  delete ui;
}

// Private methods

void
FilterDemo::applyFilter(const ImageFilter& filter) const
{
  ui->image->setPixmap(QPixmap::fromImage(filter.apply(base)));
}
ImageFilter::ColorChannel
FilterDemo::optColor() const
{
  return static_cast<ImageFilter::ColorChannel>(
    3 - ui->optionColor->currentIndex());
}

ImageFilter::Directions
FilterDemo::optDirections() const
{
  return static_cast<ImageFilter::Directions>(
    ui->optionDirection->currentIndex());
}

double
FilterDemo::optFloat() const
{
  return ui->optionFloat->value();
}

int
FilterDemo::optInt() const
{
  return ui->optionInt->value();
}

void
FilterDemo::useFloat(double min, double init, double max) const
{
  ui->optionColor->setCurrentIndex(0);
  ui->optionColor->show();
  ui->optionDirection->hide();
  ui->optionInt->hide();
  ui->optionFloat->setMinimum(min);
  ui->optionFloat->setMaximum(max);
  ui->optionFloat->setValue(init);
  ui->optionFloat->show();
}

void
FilterDemo::useInt(int min, int init, int max) const
{
  ui->optionColor->setCurrentIndex(0);
  ui->optionColor->show();
  ui->optionDirection->hide();
  ui->optionFloat->hide();
  ui->optionInt->setMinimum(min);
  ui->optionInt->setMaximum(max);
  ui->optionInt->setValue(init);
  ui->optionInt->show();
}

// Private slots

void
FilterDemo::applyFilter()
{
  if (pause) {
    return;
  }
  switch (ui->filter->currentIndex()) {
    case 0:
      applyFilter(ImageFilter::Noise(optInt()));
      return;
    case 1:
      applyFilter(ImageFilter::MonoNoise(optInt()));
      return;
    case 2:
      applyFilter(ImageFilter::Blur(optDirections()));
      return;
    case 3:
      applyFilter(ImageFilter::Sharpen(optDirections()));
      return;
    case 4:
      applyFilter(ImageFilter::EdgeDetect(optDirections()));
      return;
    case 5:
      applyFilter(ImageFilter::Emboss(optDirections()));
      return;
    case 6:
      applyFilter(ImageFilter::BrightnessAdd(optInt(), optColor()));
      return;
    case 7:
      applyFilter(ImageFilter::Contrast(optFloat(), optColor()));
      return;
    case 8:
      applyFilter(ImageFilter::Gamma(optFloat(), optColor()));
      return;
    case 9:
      applyFilter(ImageFilter::GrayscaleLinear());
      return;
    case 10:
      applyFilter(ImageFilter::GrayscalePerceptual());
      return;
    case 11:
      applyFilter(ImageFilter::BrightnessMult(optFloat(), optColor()));
      return;
    case 12:
      applyFilter(ImageFilter::LesserBlur(optDirections()));
      return;
    case 13:
      applyFilter(ImageFilter::MinorBlur(optDirections()));
      return;
    case 14:
      applyFilter(ImageFilter::Average());
      return;
    default:
      return;
  }
}

void
FilterDemo::on_filter_currentIndexChanged(int index)
{
  pause = true;
  switch (index) {
    case 0: // color noise
    case 1: // monochrome noise
      useInt(0, 100, 255);
      ui->optionColor->hide();
      break;
    case 6: // brightness (additive)
      useInt(-255, 50, 255);
      break;
    case 7: // contrast
      useFloat(-10, 1.5, 10);
      break;
    case 8: // gamma
      useFloat(0, 2, 25);
      break;
    case 11: // brightness (multiplicative)
      useFloat(0, 1.2, 10);
      break;
    case 2:  // blur
    case 3:  // sharpen
    case 4:  // edge-detect
    case 5:  // emboss
    case 12: // lesser blur
    case 13: // minor blur
      ui->optionColor->hide();
      ui->optionFloat->hide();
      ui->optionInt->hide();
      ui->optionDirection->setCurrentIndex(0);
      ui->optionDirection->show();
      break;
    default: // 9: grayscale (linear), grayscale (perceptual), average
      ui->optionColor->hide();
      ui->optionDirection->hide();
      ui->optionFloat->hide();
      ui->optionInt->hide();
  }
  pause = false;
  applyFilter();
}

void
FilterDemo::on_upload_clicked()
{
  const QString filePath = QFileDialog::getOpenFileName(
    this, QStringLiteral("Image"), QString(), FileFilter::image());
  if (filePath.isEmpty()) {
    return;
  }
  QPixmap filePixmap(filePath);
  if (filePixmap.isNull()) {
    return;
  }
  QPixmap scaledPixmap = filePixmap.scaledToHeight(ui->image->height());
  base.swap(scaledPixmap);
  ui->image->setFixedWidth(base.width());
  applyFilter();
}
