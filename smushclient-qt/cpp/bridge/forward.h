#ifndef FORWARD_H
#define FORWARD_H

#include <QtCore/QFlags>

class Alias;
enum class AliasOutcome;
using AliasOutcomes = QFlags<AliasOutcome>;
class Document;
enum class SendTarget;
struct SendTimer;
class Timekeeper;
class Timer;
class Trigger;
class World;

#endif
