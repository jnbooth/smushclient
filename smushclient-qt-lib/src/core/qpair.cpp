#include "smushclient-qt-lib/qpair.h"

#include <cxx-qt-lib/assertion_utils.h>

#define QPAIR_ASSERTS(firstTypeName, secondTypeName, combinedName)             \
  assert_alignment_and_size(QPair_##combinedName, {                            \
    firstTypeName first;                                                       \
    secondTypeName second;                                                     \
  });                                                                          \
                                                                               \
  static_assert(QTypeInfo<QPair_##combinedName>::isRelocatable);               \
                                                                               \
  static_assert(::std::is_copy_assignable<firstTypeName>::value);              \
  static_assert(::std::is_copy_constructible<firstTypeName>::value);           \
  static_assert(::std::is_move_constructible<firstTypeName>::value);           \
  static_assert(::std::is_copy_assignable<secondTypeName>::value);             \
  static_assert(::std::is_copy_constructible<secondTypeName>::value);          \
  static_assert(::std::is_move_constructible<secondTypeName>::value);

QPAIR_ASSERTS(QColor, QColor, QColor_QColor);
