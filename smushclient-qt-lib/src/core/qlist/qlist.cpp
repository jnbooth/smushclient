#include "smushclient-qt-lib/qlist.h"

#include <cxx-qt-lib/assertion_utils.h>

#define CXX_QT_IO_QLIST_ALIGN_AND_SIZE(name)                                   \
  assert_alignment_and_size(QList_##name, {                                    \
    ::std::size_t a0;                                                          \
    ::std::size_t a1;                                                          \
    ::std::size_t a2;                                                          \
  });

#define CXX_QT_IO_QLIST_ASSERTS(name)                                          \
  CXX_QT_IO_QLIST_ALIGN_AND_SIZE(name);                                        \
                                                                               \
  static_assert(!::std::is_trivially_copy_assignable<QList_##name>::value);    \
  static_assert(!::std::is_trivially_copy_constructible<QList_##name>::value); \
  static_assert(!::std::is_trivially_destructible<QList_##name>::value);       \
                                                                               \
  static_assert(QTypeInfo<QList_##name>::isRelocatable);                       \
                                                                               \
  static_assert(::std::is_copy_assignable<name>::value);                       \
  static_assert(::std::is_copy_constructible<name>::value);

CXX_QT_IO_QLIST_ASSERTS(QBrush);
CXX_QT_IO_QLIST_ASSERTS(QTextCharFormat);
CXX_QT_IO_QLIST_ASSERTS(QTextFormat);
CXX_QT_IO_QLIST_ASSERTS(QTextLength);
