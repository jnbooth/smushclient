#pragma once
#include <QtGui/QPixmap>

class ImageFilter
{
public:
  enum struct Channel
  {
    Blue = 0,
    Green = 1,
    Red = 2,
    All = 4,
  };

  virtual ~ImageFilter() {};
  virtual void apply(QPixmap &pixmap) const = 0;
};

class BrightnessAddFilter : public ImageFilter
{
public:
  inline explicit constexpr BrightnessAddFilter(int add, Channel channel = Channel::All)
      : add(add), channel(channel) {}

  void apply(QPixmap &pixmap) const override;

private:
  int add;
  Channel channel;
};

class BrightnessMultFilter : public ImageFilter
{
public:
  inline explicit constexpr BrightnessMultFilter(float multiply, Channel channel = Channel::All)
      : channel(channel), multiply(multiply) {}

  void apply(QPixmap &pixmap) const override;

private:
  Channel channel;
  float multiply;
};

class GrayscaleFilter : public ImageFilter
{
public:
  inline constexpr GrayscaleFilter() {};

  void apply(QPixmap &pixmap) const override;
};
