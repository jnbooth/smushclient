#pragma once
#include "channel.h"
#include <QtGui/QPixmap>

class ImageFilter
{
public:
  virtual ~ImageFilter() = default;
  virtual void apply(QPixmap& pixmap) const = 0;
};

class BrightnessAddFilter : public ImageFilter
{
public:
  explicit constexpr BrightnessAddFilter(
    int add,
    ColorChannel channel = ColorChannel::All)
    : add(add)
    , channel(channel)
  {
  }

  void apply(QPixmap& pixmap) const override;

private:
  int add;
  ColorChannel channel;
};

class BrightnessMultFilter : public ImageFilter
{
public:
  explicit constexpr BrightnessMultFilter(
    qreal multiply,
    ColorChannel channel = ColorChannel::All)
    : channel(channel)
    , multiply(multiply)
  {
  }

  void apply(QPixmap& pixmap) const override;

private:
  ColorChannel channel;
  qreal multiply;
};

class GrayscaleFilter : public ImageFilter
{
public:
  constexpr GrayscaleFilter() = default;

  void apply(QPixmap& pixmap) const override;
};
