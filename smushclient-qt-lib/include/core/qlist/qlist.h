#pragma once

#include <cxx-qt-lib/qlist.h>

#include "qlist_qbrush.h"
#include "qlist_qtextcharformat.h"
#include "qlist_qtextformat.h"
#include "qlist_qtextlength.h"

using QList_QBrush = QList<QBrush>;

using QList_QTextCharFormat = QList<QTextCharFormat>;
using QList_QTextCharFormatVerticalAlignment =
  QList<QTextCharFormat::VerticalAlignment>;

using QList_QTextFormat = QList<QTextFormat>;
using QList_QTextFormatFormatType = QList<QTextFormat::FormatType>;
using QList_QTextFormatObjectTypes = QList<QTextFormat::ObjectTypes>;
using QList_QTextFormatPageBreakFlag = QList<QTextFormat::PageBreakFlag>;
using QList_QTextFormatPageBreakFlags = QList<QTextFormat::PageBreakFlags>;
using QList_QTextFormatProperty = QList<QTextFormat::Property>;

using QList_QTextLength = QList<QTextLength>;
using QList_QTextLengthType = QList<QTextLength::Type>;
