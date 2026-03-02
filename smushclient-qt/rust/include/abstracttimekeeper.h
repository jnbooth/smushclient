#pragma once

#include <QtCore/QObject>

class AbstractTimekeeper : public QObject
{
public:
  explicit AbstractTimekeeper(QObject* parent = nullptr)
    : QObject(parent)
  {
  }

  virtual void startSendTimer(size_t index,
                              uint16_t timerId,
                              unsigned int millis) const = 0;
};
