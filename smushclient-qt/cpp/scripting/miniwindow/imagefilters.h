#pragma once
#include "smushclient_qt/src/ffi/filter.cxx.h"
#include <QtGui/QPixmap>

class ImageFilter
{
public:
  using ColorChannel = ffi::filter::ColorChannel;
  using Directions = ffi::filter::Directions;
  using Pixels = rust::Slice<uint32_t>;

  virtual ~ImageFilter() = default;
  [[nodiscard]] virtual QImage apply(const QPixmap& pixmap) const noexcept = 0;

public:
  class Noise;
  class MonoNoise;
  class Blur;
  class Sharpen;
  class EdgeDetect;
  class Emboss;
  class BrightnessAdd;
  class Contrast;
  class Gamma;
  class GrayscaleLinear;
  class GrayscalePerceptual;
  class BrightnessMult;
  class LesserBlur;
  class MinorBlur;
  class Average;

protected:
  class PixelFilter;
  class ConvolveFilter;
};

class ImageFilter::PixelFilter : public ImageFilter
{
public:
  QImage apply(const QPixmap& pixmap) const noexcept override;

protected:
  virtual void apply(Pixels pixels) const noexcept = 0;
  virtual bool isNoop() const noexcept { return false; }
};

class ImageFilter::ConvolveFilter : public ImageFilter
{
public:
  explicit constexpr ConvolveFilter(Directions directions) noexcept
    : directions(directions)
  {
  }
  QImage apply(const QPixmap& pixmap) const noexcept override;

protected:
  virtual void apply(Pixels pixels,
                     int width,
                     Directions directions) const noexcept = 0;

private:
  Directions directions;
};

// NOLINTBEGIN(bugprone-macro-parentheses)
#define CONVOLVE_FILTER(name)                                                  \
  class ImageFilter::name : public ConvolveFilter                              \
  {                                                                            \
  public:                                                                      \
    explicit constexpr name(Directions directions = Directions::Both) noexcept \
      : ConvolveFilter(directions)                                             \
    {                                                                          \
    }                                                                          \
                                                                               \
  protected:                                                                   \
    void apply(Pixels pixels,                                                  \
               int width,                                                      \
               Directions directions) const noexcept override;                 \
  };
// NOLINTEND(bugprone-macro-parentheses)

class ImageFilter::Noise : public PixelFilter
{
public:
  explicit constexpr Noise(qreal threshold) noexcept
    : threshold(threshold)
  {
  }

protected:
  void apply(Pixels pixels) const noexcept override;
  constexpr bool isNoop() const noexcept override { return threshold == 0.0; }

private:
  qreal threshold;
};

class ImageFilter::MonoNoise : public PixelFilter
{
public:
  explicit constexpr MonoNoise(qreal threshold) noexcept
    : threshold(threshold)
  {
  }

protected:
  void apply(Pixels pixels) const noexcept override;
  constexpr bool isNoop() const noexcept override { return threshold == 0.0; }

private:
  qreal threshold;
};

CONVOLVE_FILTER(Blur)

CONVOLVE_FILTER(Sharpen)

CONVOLVE_FILTER(EdgeDetect)

CONVOLVE_FILTER(Emboss)

class ImageFilter::BrightnessAdd : public PixelFilter
{
public:
  explicit constexpr BrightnessAdd(
    int add,
    ColorChannel channel = ColorChannel::All) noexcept
    : add(add)
    , channel(channel)
  {
  }

protected:
  void apply(Pixels pixels) const noexcept override;
  constexpr bool isNoop() const noexcept override { return add == 0; }

private:
  int add;
  ColorChannel channel;
};

class ImageFilter::Contrast : public PixelFilter
{
public:
  explicit constexpr Contrast(qreal multiply,
                              ColorChannel channel = ColorChannel::All) noexcept
    : channel(channel)
    , multiply(multiply)
  {
  }

protected:
  void apply(Pixels pixels) const noexcept override;
  constexpr bool isNoop() const noexcept override { return multiply == 1.0; }

private:
  ColorChannel channel;
  qreal multiply;
};

class ImageFilter::Gamma : public PixelFilter
{
public:
  explicit constexpr Gamma(qreal exp,
                           ColorChannel channel = ColorChannel::All) noexcept
    : channel(channel)
    , exp(exp)
  {
  }

protected:
  void apply(Pixels pixels) const noexcept override;
  constexpr bool isNoop() const noexcept override { return exp == 1.0; }

private:
  ColorChannel channel;
  qreal exp;
};

class ImageFilter::GrayscaleLinear : public PixelFilter
{
public:
  void apply(Pixels pixels) const noexcept override;
};

class ImageFilter::GrayscalePerceptual : public PixelFilter
{
public:
  void apply(Pixels pixels) const noexcept override;
};

class ImageFilter::BrightnessMult : public PixelFilter
{
public:
  explicit constexpr BrightnessMult(
    qreal multiply,
    ColorChannel channel = ColorChannel::All) noexcept
    : channel(channel)
    , multiply(multiply)
  {
  }

protected:
  void apply(Pixels pixels) const noexcept override;
  constexpr bool isNoop() const noexcept override { return multiply == 1.0; }

private:
  ColorChannel channel;
  qreal multiply;
};

CONVOLVE_FILTER(LesserBlur)

CONVOLVE_FILTER(MinorBlur)

class ImageFilter::Average : public PixelFilter
{
public:
  void apply(Pixels pixels) const noexcept override;
};

#undef CONVOLVE_FILTER
