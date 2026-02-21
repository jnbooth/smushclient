#pragma once
#include "smushclient_qt/src/ffi/client.cxxqt.h"

struct TimekeeperItem;

class SmushClient : public SmushClientBase
{
  Q_OBJECT

public:
  explicit SmushClient(QObject* parent = nullptr);

  bool finishTimer(const TimekeeperItem& item);

public slots:
  void onTimersPolled() noexcept;
};
