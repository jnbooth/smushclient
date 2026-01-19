#pragma once

namespace native {
enum SetBackgroundMaterialResult
{
  OK,
  Unsupported,
  PassedNullPointer,
  NoSuperview,
  InvalidMaterial,
};
} // namespace native
