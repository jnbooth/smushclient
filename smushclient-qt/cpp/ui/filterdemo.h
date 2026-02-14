#ifndef FILTERDEMO_H
#define FILTERDEMO_H

#include "smushclient_qt/src/ffi/filter.cxx.h"
#include <QtWidgets/QDialog>

namespace Ui {
class FilterDemo;
} // namespace Ui

class ImageFilter;

class FilterDemo : public QDialog
{
  Q_OBJECT

public:
  explicit FilterDemo(QWidget* parent = nullptr);
  ~FilterDemo() override;

private:
  void applyFilter(const ImageFilter& filter) const;
  ffi::filter::ColorChannel optColor() const;
  ffi::filter::Directions optDirections() const;
  double optFloat() const;
  int optInt() const;
  void useFloat(double min, double init, double max) const;
  void useInt(int min, int init, int max) const;

private slots:
  void applyFilter();
  void on_filter_currentIndexChanged(int index);
  void on_optionColor_currentIndexChanged(int /* idx */) { applyFilter(); }
  void on_optionDirection_currentIndexChanged(int /* idx */) { applyFilter(); }
  void on_optionFloat_valueChanged(double /* val */) { applyFilter(); }
  void on_optionInt_valueChanged(int /* val */) { applyFilter(); }
  void on_upload_clicked();

private:
  Ui::FilterDemo* ui;
  QPixmap base;
  bool pause = false;
};

#endif // FILTERDEMO_H
