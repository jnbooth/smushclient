#ifndef FORWARD_H
#define FORWARD_H

#include <QtCore/QFlags>
#include <QtCore/QVector>
#include <QtGui/QColor>

class Alias;
class Timer;
class Trigger;

enum class AliasOutcome : uint8_t;
enum class SendTarget;

class Document;
class SmushClientBase;
class Timekeeper;
class World;

using AliasOutcomes = QFlags<AliasOutcome>;
using QVector_QColor = QVector<QColor>;

#endif
