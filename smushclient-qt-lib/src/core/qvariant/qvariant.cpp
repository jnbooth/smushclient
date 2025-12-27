#include "smushclient-qt-lib/qvariant.h"

#define CXX_QT_QVARIANT_CAN_CONVERT_IMPL(typeName, name)                       \
  bool qvariantCanConvert##name(const QVariant& variant)                       \
  {                                                                            \
    return variant.canConvert<typeName>();                                     \
  }

namespace rust {
namespace cxxqtlib1 {
namespace qvariant {

CXX_QT_QVARIANT_CAN_CONVERT_IMPL(::QBrush, QBrush)
CXX_QT_QVARIANT_CAN_CONVERT_IMPL(::QTextFormat, QTextFormat)
CXX_QT_QVARIANT_CAN_CONVERT_IMPL(::QTextLength, QTextLength)

} // namespace qvariant
} // namespace cxxqtlib1
} // namespace rust
