#include "imagebits.h"

// Private utils

namespace {
constexpr int
rgbaOffset(ColorChannel channel) noexcept
{
  if (channel == ColorChannel::All) {
    return 0;
  }
  if constexpr (std::endian::native == std::endian::big) {
    return 2 - channel; // A R G B
  } else {
    return channel; // B G R A
  }
}
} // namespace

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)

ImageBitsRange::ImageBitsRange(QImage& image, ColorChannel channel)
  : beginPtr(image.bits() + rgbaOffset(channel))
  , endPtr(beginPtr + image.sizeInBytes())
  , bitshift(channel == ColorChannel::All ? 0 : 2) {};

// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
