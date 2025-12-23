#include "smushclient-qt-lib/qmap.h"

#include <cxx-qt-lib/assertion_utils.h>

#define CXX_QT_IO_QMAP_ASSERTS(keyTypeName, valueTypeName, combinedName)       \
  assert_alignment_and_size(QMap_##combinedName, { ::std::size_t a0; });       \
                                                                               \
  static_assert(                                                               \
      !::std::is_trivially_copy_assignable<QMap_##combinedName>::value);       \
  static_assert(                                                               \
      !::std::is_trivially_copy_constructible<QMap_##combinedName>::value);    \
  static_assert(                                                               \
      !::std::is_trivially_destructible<QMap_##combinedName>::value);          \
                                                                               \
  static_assert(QTypeInfo<QMap_##combinedName>::isRelocatable);                \
                                                                               \
  static_assert(::std::is_copy_assignable<keyTypeName>::value);                \
  static_assert(::std::is_copy_constructible<keyTypeName>::value);             \
  static_assert(::std::is_copy_assignable<valueTypeName>::value);              \
  static_assert(::std::is_copy_constructible<valueTypeName>::value);

CXX_QT_IO_QMAP_ASSERTS(::std::int32_t, QVariant, i32_QVariant);
