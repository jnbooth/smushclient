#pragma once
#include <QtGui/QPixmap>

class ImageFilter
{
public:
  enum Channel : int
  {
    Blue = 0,
    Green = 1,
    Red = 2,
    All = 4,
  };

  virtual ~ImageFilter() = default;
  virtual void apply(QPixmap& pixmap) const = 0;
};

class BrightnessAddFilter : public ImageFilter
{
public:
  explicit constexpr BrightnessAddFilter(int add,
                                         Channel channel = Channel::All)
    : add(add)
    , channel(channel)
  {
  }

  void apply(QPixmap& pixmap) const override;

private:
  int add;
  Channel channel;
};

class BrightnessMultFilter : public ImageFilter
{
public:
  explicit constexpr BrightnessMultFilter(qreal multiply,
                                          Channel channel = Channel::All)
    : channel(channel)
    , multiply(multiply)
  {
  }

  void apply(QPixmap& pixmap) const override;

private:
  Channel channel;
  qreal multiply;
};

class GrayscaleFilter : public ImageFilter
{
public:
  constexpr GrayscaleFilter() = default;

  void apply(QPixmap& pixmap) const override;
};
