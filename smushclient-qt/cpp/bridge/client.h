#pragma once
#include <smushclient_qt/src/ffi/client.cxxqt.h>

struct TimekeeperItem;

class SmushClient : public SmushClientBase
{
  Q_OBJECT

public:
  using SmushClientBase::SmushClientBase;

  bool finishTimer(const TimekeeperItem& item);

  using SmushClientBase::getMetavariable;

  template<typename T>
  std::optional<T> getMetavariable(rust::string_view key) const
  {
    const QByteArray bytes(SmushClientBase::getMetavariable(key));
    if (bytes.isEmpty()) {
      return std::nullopt;
    }
    T data;
    QDataStream stream(bytes);
    stream >> data;
    if (stream.status() != QDataStream::Status::Ok) {
      return std::nullopt;
    }
    return data;
  }

  using SmushClientBase::setMetavariable;

  template<typename T>
  bool setMetavariable(rust::string_view key, const T& data) const
  {
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream << data;
    SmushClientBase::setMetavariable(key, bytes);
    return stream.status() == QDataStream::Status::Ok;
  }

public slots:
  void onTimersPolled() noexcept;
};
