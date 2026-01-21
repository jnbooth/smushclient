#ifndef FORWARD_H
#define FORWARD_H

#include <QtCore/QFlags>

class Alias;
class Timer;
class Trigger;

enum class AliasOutcome;
enum class SendTarget;

class Document;
class SmushClientBase;
class Timekeeper;
class World;

using AliasOutcomes = QFlags<AliasOutcome>;

#endif
